#ifndef INC_IOSTATUS_H_
#define INC_IOSTATUS_H_

#include <windows.h>			// POINT
#include "xbox_controller.h"


class CIOStatus {
public:
	CXBOXController* Player1;

	struct TAnalogStick {
		float x,y;				// -1 .. 1
		float normalized_magnitude; // 0..1
	};

	TAnalogStick left;
	TAnalogStick right;

	float getSpeed() const;
	bool isSprinting() const;
	bool isPlayer1Connected() const;

	struct TDigital {
		bool is_pressed;
		bool was_pressed;
		float time_pressed;

		TDigital( ) 
			: is_pressed( false )
			, was_pressed( false )
			, time_pressed( 0.0f )
		{} 
		bool isPressed() const { return is_pressed; }
		bool isReleased() const { return !is_pressed; }
		bool becomesPressed() const { return is_pressed && !was_pressed; }
		bool becomesReleased() const { return !is_pressed && was_pressed; }
	};

	

	enum TButton {
		CREATE_SHADOW = 0
	,	SILENT_KILL // el mismo boton
	,   AIM
	,	DECOY
	,	BLEND
	,	RECHARGE
	,	SPECIAL_VISION
	,	CROW
	,	FIGHT
	,	TELEPORT	// el mismo boton
	,	FORWARD	
	,	BACKWARD
	,	LEFT
	,	RIGHT
	,	SPRINT

	,   MOUSE_MIDDLE
	,   MOUSE_WHEEL_UP
	,   MOUSE_WHEEL_DOWN
	,	KEYBOARD_ALT
	,	KEYBOARD_F
	,	KEYBOARD_CONTROL
	,	KEYBOARD_Z  //De momento hace de Control
	,	KEYBOARD_T

	,   LEFT_STICK
	,   RIGHT_STICK
	,   D_UP
	,   D_DOWN
	,   D_LEFT
	,   D_RIGHT
	,	START
	,	BACK
	,	PRUEBAS

	,   BUTTONS_COUNT
	};


	static CIOStatus*	instance();
	static void		deleteInstance();

	// Globals to be accesses from win_main WM_MOUSE_* handler
	bool  current_mouse_left;
	bool  current_mouse_right;
	bool  current_mouse_middle;
	int   current_mouse_wheel;
	int currentKey;
	float currentKey_time_pressed;

	TDigital buttons[ BUTTONS_COUNT ];
	POINT prev_mouse_loc;
	POINT curr_mouse_loc;
	POINT delta_mouse;
	bool  prev_mouse_loc_is_valid;
	bool  has_control_of_mouse;

	void releaseMouse( );
	void adquireMouse( );
	void update( float delta );
	bool isPressed( TButton b ) const;
	bool becomesPressed( TButton b ) const;
	bool becomesReleased( TButton b ) const;
	bool isReleased( TButton b ) const;
	float getHorizontalAxis(void);
	float getVerticalAxis(void);
	bool theresMovement();

	bool isPressed( int key ) const;
	bool becomesPressed_key( int key );

	void vibrate( int left, int right );

protected:
	CIOStatus();
	CIOStatus(const CIOStatus &);
	CIOStatus &operator= (const CIOStatus &);
	~CIOStatus( );
private:
	static CIOStatus*		_instance;
};

#endif
