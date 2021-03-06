﻿#include "SinusOnJack.h"

SinusOnJack::SinusOnJack(QObject *parent) :
	QThread(parent)
{
	m_clientOpened = false;
	m_clientActived = false;
	m_client = NULL;
	m_continue = false;
	m_cycle = 0;
	m_offset = 0;
	m_portAudioOutLeft = NULL;
	m_portAudioOutRight = NULL;
	m_portMidiIn = NULL;
	m_sampleInCycle = 0;
	m_sampleRate = 0;
	m_tune = 440;
	m_velocityEnabled = true;
	m_waveType = Sinusoide;
	m_gain = 1.0;
	int i;
	for(i = 0; i < 128; ++i)
	{
		m_noteFrqs[i] = (2.0 * m_tune / 32.0) * pow( 2, (((jack_default_audio_sample_t)i - 9.0) / 12.0)) / m_sampleRate;
	}
	m_ramp = 0.0;
}

SinusOnJack::~SinusOnJack()
{
	stop();
}

int SinusOnJack::staticProcess(jack_nframes_t nframes, void* arg)
{
	return static_cast<SinusOnJack*>(arg)->process(nframes);
}

int SinusOnJack::process(jack_nframes_t nframes)
{
	//cout << nframes << " échantillons à traiter." << endl;

	m_sampleInCycle = nframes;
	int i;
	void* portMidiInBuffer = jack_port_get_buffer(m_portMidiIn, nframes);
	jack_default_audio_sample_t* outLeft =
			(jack_default_audio_sample_t*)jack_port_get_buffer(m_portAudioOutLeft,
																												 nframes);
	jack_default_audio_sample_t* outRight =
			(jack_default_audio_sample_t*)jack_port_get_buffer(m_portAudioOutRight,
																												 nframes);
	jack_midi_event_t inEvent;
	jack_nframes_t eventIndex = 0;
	jack_nframes_t eventCount = jack_midi_get_event_count(portMidiInBuffer);
	for(i = 0; i < nframes; ++i)
	{
		if (eventIndex < eventCount)
		{
			cout << "Lecture du message " << eventIndex << endl;
			jack_midi_event_get(&inEvent,
													portMidiInBuffer,
													0);
			if(((*(inEvent.buffer) & 0xf0)) == 0x90)
			{
				m_note = *(inEvent.buffer + 1);
				m_noteOn = (m_velocityEnabled)?*(inEvent.buffer + 2)/127.0:1.0;
				cout << "Note On : " << (int)m_note << " : " << m_noteOn << "//";
			}
			else if(((*(inEvent.buffer)) & 0xf0) == 0x80)
			{
				m_note = *(inEvent.buffer + 1);
				m_noteOn = 0.0;
				cout << "Note Off : " << (int)m_note << "//";
			}
			++eventIndex;
		}
		m_ramp += m_noteFrqs[m_note];
		m_ramp = (m_ramp > 1.0) ? m_ramp - 2.0 : m_ramp;
		jack_default_audio_sample_t sample = m_noteOn*sin(2*M_PI*m_ramp) * m_gain;
		if (sample > m_currentSample + 0.1) sample = m_currentSample + 0.1;
		if (sample < m_currentSample - 0.1) sample = m_currentSample - 0.1;
		m_currentSample = sample;
		outLeft[i] = m_currentSample;
		outRight[i] = outLeft[i];
	}
	return 0;
}

int SinusOnJack::staticSampleRate(jack_nframes_t nframes, void* arg)
{
	return static_cast<SinusOnJack*>(arg)->sampleRate(nframes);
}

int SinusOnJack::sampleRate(jack_nframes_t nframes)
{
	m_sampleRate = nframes;
	int i;
	for(i = 0; i < 128; ++i)
	{
		m_noteFrqs[i] = (2.0 * 440.0 / 32.0) * pow( 2, (((jack_default_audio_sample_t)i - 9.0) / 12.0)) / m_sampleRate;
	}
	return 0;
}

void SinusOnJack::staticError(const char* desc)
{
	cout << desc << endl;
}

void SinusOnJack::staticJackShutdown(void* arg)
{
	static_cast<SinusOnJack*>(arg)->jackShutdown();
}

void SinusOnJack::jackShutdown()
{

}

void SinusOnJack::run()
{
	m_continue = true;

	// Gestion des erreurs
	jack_set_error_function(SinusOnJack::staticError);

	// Ouverture du client
	m_client = jack_client_open("QSinJack",
															JackServerName,
															&m_status,
															"default");
	if (m_client)
		{
			m_clientOpened = true;
			emit clientIsOpen();
		}
	else
		{
			m_clientOpened = false;
			emit clientIsNotOpen();
		}
	emit clientOpened(m_clientOpened);

	jack_set_process_callback(m_client, SinusOnJack::staticProcess, this);
	jack_set_sample_rate_callback(m_client, SinusOnJack::staticSampleRate, this);
	jack_on_shutdown(m_client, SinusOnJack::staticJackShutdown, this);

	m_sampleRate = jack_get_sample_rate(m_client);
	m_portMidiIn = jack_port_register(m_client,
																		"Input",
																		JACK_DEFAULT_MIDI_TYPE,
																		JackPortIsInput|
																		JackPortIsTerminal,
																		0);
	m_portAudioOutLeft = jack_port_register(m_client,
																					"Left Output",
																					JACK_DEFAULT_AUDIO_TYPE,
																					JackPortIsOutput|
																					JackPortIsTerminal,
																					0);
	m_portAudioOutRight = jack_port_register(m_client,
																					 "Right Output",
																					 JACK_DEFAULT_AUDIO_TYPE,
																					 JackPortIsOutput|
																					 JackPortIsTerminal,
																					 0);

	int error;
	error = jack_activate(m_client);
	if (error == 0)
		{
			m_clientActived = true;
			emit clientIsActive();
		}
	else
		{
			m_clientActived = false;
			emit clientIsNotActive();
		}
	emit clientActived(m_clientActived);

	while (m_continue)
	{
		wait(1);
	}

	jack_deactivate(m_client);
	jack_port_unregister(m_client,
											 m_portAudioOutLeft);
	jack_client_close(m_client);
}

void SinusOnJack::stop()
{
	m_continue = false;
}

void SinusOnJack::updateGain(int gain)
{
	m_gain = gain / 100.0;
}
