#ifndef SINUSONJACK_H
#define SINUSONJACK_H

#define M_PI 3.14159265359

#include <QThread>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <iostream>
#include <cmath>

using namespace std;

class SinusOnJack : public QThread
{
	Q_OBJECT

	// Enumération
public:
	enum
	{
		Sinusoide,
		Carre,
		Triangle,
		DentDeScie
	};
	// Variables
private:
	// Client et Ports
	jack_client_t* m_client;
	bool m_clientOpened;
	bool m_clientActived;
	jack_status_t m_status;
	jack_port_t* m_portMidiIn;
	jack_port_t* m_portAudioOutLeft;
	jack_port_t* m_portAudioOutRight;

	// Paramètre Jack
	jack_nframes_t m_sampleRate;
	jack_nframes_t m_sampleInCycle;
	jack_default_audio_sample_t* m_cycle;

	// Paramètre du synthétiseur
	int m_tune;
	long m_offset;
	unsigned char m_note;
	jack_default_audio_sample_t m_ramp;
	jack_default_audio_sample_t m_currentSample;
	jack_default_audio_sample_t m_noteOn;
	jack_default_audio_sample_t m_noteFrqs[128];

	// Variable du son
	int m_waveType;
	bool m_limitEnabled;

	// Controle physique
	jack_default_audio_sample_t m_gain;
	jack_default_audio_sample_t m_gain_left;
	jack_default_audio_sample_t m_gain_right;
	jack_default_audio_sample_t m_balance;
	jack_default_audio_sample_t m_pan;
	jack_default_audio_sample_t m_low;
	jack_default_audio_sample_t m_lowCut;
	jack_default_audio_sample_t m_medium;
	jack_default_audio_sample_t m_mediumCut;
	jack_default_audio_sample_t m_high;
	jack_default_audio_sample_t m_highCut;
	jack_default_audio_sample_t m_reverb;
	jack_default_audio_sample_t m_chorus;
	jack_default_audio_sample_t m_effect1;
	jack_default_audio_sample_t m_effect2;

	// Réactivité aux messages MIDI
	bool m_velocityEnabled;
	bool m_polyEnabled;
	bool m_pitchBendEnabled;

	// Paramètre du thread
	bool m_continue;

	// Constructeur/Destructeur
public:
	explicit SinusOnJack(QObject *parent = 0);
	~SinusOnJack();
	static int staticProcess(jack_nframes_t nframes, void* arg);
	int process(jack_nframes_t nframes);
	static int staticSampleRate(jack_nframes_t nframes, void* arg);
	int sampleRate(jack_nframes_t nframes);
	static void staticError(const char* desc);
	static void staticJackShutdown(void* arg);
	void jackShutdown();

	// Méthodes
private:

protected:
	void run();

signals:
	void clientIsOpen();
	void clientIsNotOpen();
	void clientOpened(bool);
	void clientIsActive();
	void clientIsNotActive();
	void clientActived(bool);

public slots:
	void stop();
	void updateGain(int gain);

};

#endif // SINUSONJACK_H
