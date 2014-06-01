#include "SinusOnJack.h"

SinusOnJack::SinusOnJack(QObject *parent) :
	QThread(parent)
{
	m_clientIsOpen = false;
	m_clientIsActive = false;
	m_client = NULL;
	m_continue = false;
	m_cycle = 0;
	m_offset = 0;
	m_portAudioOut = NULL;
	m_portMidiIn = NULL;
	m_sampleInCycle = 0;
	m_sampleRate = 0;
	m_tone = 440;
	m_velocityEnabled = true;
	int i;
	for(i = 0; i < 128; ++i)
	{
		m_noteFrqs[i] = (2.0 * 440.0 / 32.0) * pow( 2, (((jack_default_audio_sample_t)i - 9.0) / 12.0)) / m_sampleRate;
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
	jack_default_audio_sample_t* out =
			(jack_default_audio_sample_t*)jack_port_get_buffer(m_portAudioOut,
																												 nframes);
	jack_midi_event_t inEvent;
	jack_nframes_t eventIndex = 0;
	jack_nframes_t eventCount = jack_midi_get_event_count(portMidiInBuffer);
	jack_midi_event_get(&inEvent,
											portMidiInBuffer,
											0);
	for(i = 0; i < nframes; ++i)
	{
		if(((*(inEvent.buffer) & 0xf0)) == 0x90)
		{
			m_note = *(inEvent.buffer + 1);
			m_noteOn = (m_velocityEnabled)?*(inEvent.buffer + 2)/127.0:1.0;
			cout << "Note On : " << (int)m_note << " : " << m_noteOn << endl;
		}
		else if(((*(inEvent.buffer)) & 0xf0) == 0x80)
		{
			m_note = *(inEvent.buffer + 1);
			m_noteOn = 0.0;
			cout << "Note Off : " << (int)m_note << endl;
		}
		++eventIndex;
		jack_midi_event_get(&inEvent,
												portMidiInBuffer,
												eventIndex);
		m_ramp += m_noteFrqs[m_note];
		m_ramp = (m_ramp > 1.0) ? m_ramp - 2.0 : m_ramp;
		out[i] = m_noteOn*sin(2*M_PI*m_ramp);
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
	m_client = jack_client_open("MidiTest",
															JackServerName,
															&m_status,
															"default");
	if (m_client)
		{
			m_clientIsOpen = true;
			emit clientIsOpen();
		}
	else
		{
			m_clientIsOpen = false;
			emit clientIsNotOpen();
		}
	emit clientOpened(m_clientIsOpen);

	jack_set_process_callback(m_client, SinusOnJack::staticProcess, this);
	jack_set_sample_rate_callback(m_client, SinusOnJack::staticSampleRate, this);
	jack_on_shutdown(m_client, SinusOnJack::staticJackShutdown, this);

	m_sampleRate = jack_get_sample_rate(m_client);
	m_portAudioOut = jack_port_register(m_client,
																			"Output",
																			JACK_DEFAULT_AUDIO_TYPE,
																			JackPortIsOutput|
																			JackPortIsTerminal,
																			0);
	m_portMidiIn = jack_port_register(m_client,
																		"Input",
																		JACK_DEFAULT_MIDI_TYPE,
																		JackPortIsInput|
																		JackPortIsTerminal,
																		0);

	int error;
	error = jack_activate(m_client);
	if (error == 0)
		{
			m_clientIsActive = true;
			emit clientIsActive();
		}
	else
		{
			m_clientIsActive = false;
			emit clientIsNotActive();
		}
	emit clientActived(m_clientIsActive);

	while (m_continue)
	{
		wait(1);
	}

	jack_deactivate(m_client);
	jack_port_unregister(m_client,
											 m_portAudioOut);
	jack_client_close(m_client);
}

void SinusOnJack::stop()
{
	m_continue = false;
}
