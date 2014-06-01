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

	// Variables
private:
	jack_client_t* m_client;
	jack_status_t m_status;
	jack_port_t* m_portAudioOut;
	jack_port_t* m_portMidiIn;
	jack_default_audio_sample_t* m_cycle;
	jack_nframes_t m_sampleRate;
	jack_nframes_t m_sampleInCycle;
	long m_offset;
	int m_tone;
	unsigned char m_note;
	jack_default_audio_sample_t m_ramp;
	jack_default_audio_sample_t m_noteOn;
	jack_default_audio_sample_t m_noteFrqs[128];
	bool m_velocityEnabled;

	bool m_clientIsOpen;
	bool m_clientIsActive;

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

};

#endif // SINUSONJACK_H
