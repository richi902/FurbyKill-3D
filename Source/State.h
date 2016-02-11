/*
	FurbyKill 3D
	============
	License: GPL
	Author: Andre Schnabel
	E-Mail: thefrogs@web.de
	Homepage: http://www.semler-service.de/andre/
*/

// State.h

#ifndef STATE_H
#define STATE_H

class State
{
	public:
		virtual ~State() {}
		
		virtual void Init() = 0;
		virtual void Quit() = 0;
		
		virtual void Input() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;
		
		void SetDone(bool done) { this->done = done; }
		bool GetDone() const { return done; }
		
		void SetRestartMe(bool restartMe) { this->restartMe = restartMe; }
		bool GetRestartMe() const { return restartMe; }
					
	protected:
		bool restartMe;
		
	private:
		bool done;
};

#endif
