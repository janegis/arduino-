#ifndef EncButton_h
#define EncButton_h

#define _EB_FAST 30    
#define _EB_DEB 80      
#define _EB_HOLD 1000   
#define _EB_STEP 500    
#define _EB_CLICK 400	


#include <Arduino.h>
#include "FastIO.h"

#define _setFlag(x) (flags |= 1 << x)
#define _clrFlag(x) (flags &= ~(1 << x))
#define _readFlag(x) ((flags >> x) & 1)

#ifndef EB_FAST
#define EB_FAST _EB_FAST
#endif
#ifndef EB_DEB
#define EB_DEB _EB_DEB
#endif
#ifndef EB_HOLD
#define EB_HOLD _EB_HOLD
#endif
#ifndef EB_STEP
#define EB_STEP _EB_STEP
#endif
#ifndef EB_CLICK
#define EB_CLICK _EB_CLICK
#endif

enum eb_callback {
	TURN_HANDLER,
	RIGHT_HANDLER,
	LEFT_HANDLER,
	RIGHT_H_HANDLER,
	LEFT_H_HANDLER,
	CLICK_HANDLER,
	HOLDED_HANDLER,
	STEP_HANDLER,
	HOLD_HANDLER,
	CLICKS_HANDLER,
};

#define EB_TICK 0
#define EB_CALLBACK 1


template < bool MODE, uint8_t S1, uint8_t S2 = 255, uint8_t KEY = 255 >
class EncButton {
public:	
	void pullUp() {
		if (S2 == 255) {         
			pinMode(S1, INPUT_PULLUP);
		} else if (KEY == 255) { 
			pinMode(S1, INPUT_PULLUP);
			pinMode(S2, INPUT_PULLUP);
		} else {                
			pinMode(S1, INPUT_PULLUP);
			pinMode(S2, INPUT_PULLUP);
			pinMode(KEY, INPUT_PULLUP);
		}	
	}

	void tick(bool hold = 0) {
		uint32_t thisMls = millis();
		uint32_t debounce = thisMls - _debTimer;

	
		if (S1 != 255 && S2 != 255) {
			uint8_t state = fastRead(S1) | (fastRead(S2) << 1);                
			if (_readFlag(0) && state == 0b11) {                             
				if (S2 == 255 || KEY != 255) {                                 
					if (!_readFlag(4)) {                                        
						if (_lastState == 0b10) EBState = (_btnState || hold) ? 3 : 1, counter++;
						else if (_lastState == 0b01) EBState = (_btnState || hold) ? 4 : 2, counter--;
					}
				} else {                                                       
					if (_lastState == 0b10) EBState = 1, counter++;
					else if (_lastState == 0b01) EBState = 2, counter--;
				}
				if (EBState != 0) {
					if (MODE) _dir = EBState;
					
					if (debounce < EB_FAST) _setFlag(1);	
					else _clrFlag(1);									
				}
				
				_clrFlag(0);
				_debTimer = thisMls;
			}
			if (state == 0b00) _setFlag(0);
			_lastState = state;
		}

	
		if (S2 == 255 || KEY != 255) {
			if (S2 == 255) _btnState = !fastRead(S1);                       	
			if (KEY != 255) _btnState = !fastRead(KEY);                     	

			if (_btnState) {                                                
				if (!_readFlag(3)) {                                          
					if (debounce > EB_DEB) {                                   	
						_setFlag(3);                                            
						_debTimer = thisMls;                                    
						EBState = 0;                                           	
					}
					if (debounce > EB_CLICK) {									
						clicks = 0;											
						flags &= ~0b01100000;
					}
				} else {                                                      	
					if (!_readFlag(4)) {                                       
						if (debounce < EB_HOLD) {                              	
							if (EBState != 0) _setFlag(2);                     	
						} else {                                                
							if (!_readFlag(2)) {                                
								EBState = 6;                                   	
								_setFlag(4);                                    
								_debTimer = thisMls;                            
							}
						}
					} else {                                                  
						if (debounce > EB_STEP) {                              	
							EBState = 7;                                       	
							_debTimer = thisMls;                                
						}
					}
				}
			} else {                                                        
				if (_readFlag(3)) {                                           	
					if (debounce > EB_DEB && !_readFlag(4) && !_readFlag(2)) {	
						EBState = 5;
						clicks++;
					}
					flags &= ~0b00011100;                                      
					_debTimer = thisMls;                                       
				} else if (clicks > 0 && debounce > EB_CLICK && !_readFlag(5)) flags |= 0b01100000;	 
			}
		}
		if (MODE) {
			if (*_callback[0] && isTurn()) _callback[0]();
			switch (EBState) {
			case 1: if (*_callback[1]) _callback[1](); break;				
			case 2: if (*_callback[2]) _callback[2](); break;			
			case 3: if (*_callback[3]) _callback[3](); break;				
			case 4: if (*_callback[4]) _callback[4](); break;				
			case 5: if (*_callback[5]) _callback[5](); break;			
			case 6: if (*_callback[6]) _callback[6](); break;				
			case 7: if (*_callback[7]) _callback[7](); break;						
			}
			EBState = 0;
			if (*_callback[8] && _readFlag(4)) _callback[8](); 
			if (_readFlag(6)) {			
				if (*_callback[9]) _callback[9]();				
				if (*_callback[10] && clicks == _amount) _callback[10]();
				_clrFlag(6);
			}		
		}
	}

	void attach(eb_callback type, void (*handler)()) {
		_callback[type] = *handler;
	}
	void detach(eb_callback type) {
		_callback[type] = NULL;
	}
	void attachClicks(uint8_t amount, void (*handler)()) {
		_amount = amount;
		_callback[10] = *handler;
	}
	void detachClicks() {
		_callback[10] = NULL;
	}
	
	bool isRight() { return MODE ? (_dir == 1 ? 1 : 0) : checkState(1); }
	bool isLeft() { return MODE ? (_dir == 2 ? 1 : 0) : checkState(2); }
	bool isRightH() { return MODE ? (_dir == 3 ? 1 : 0) : checkState(3); }
	bool isLeftH() { return MODE ? (_dir == 4 ? 1 : 0) : checkState(4); }

	uint8_t getState() { return EBState; }
	void resetState() { EBState = 0; }
	bool isFast() { return _readFlag(1); }
	bool isTurn() { return (EBState > 0 && EBState < 5); }
	bool isClick() { return checkState(5); }
	bool isHolded() { return checkState(6); }
	bool isHold() { return _readFlag(4); }
	bool isStep() { return checkState(7); }
	bool state() { return !fastRead(S1); }
	bool hasClicks(uint8_t numClicks) {
		if (clicks == numClicks && _readFlag(6)) {
			_clrFlag(6);
			return 1;
		}
		return 0;
	}
	uint8_t hasClicks() {
		if (_readFlag(6)) {
			_clrFlag(6);
			return clicks;
		} return 0;	
	}

	int counter = 0;
	uint8_t clicks = 0;

private:
	bool checkState(uint8_t val) {
		if (EBState == val) {
			EBState = 0;
			return 1;
		} return 0;
	}
	uint32_t _debTimer = 0;
	uint8_t _lastState = 0, EBState = 0;
	bool _btnState = 0;
	uint8_t flags = 0;
	

	uint8_t _dir = 0;
	void (*_callback[MODE ? 11 : 0])() = {};
	uint8_t _amount = 0;

};

#endif