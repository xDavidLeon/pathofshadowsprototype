#include "iostatus.h"
#include "globals.h"
#include "world.h"

CIOStatus* CIOStatus::_instance = 0;

CIOStatus* CIOStatus::instance ()
{
  if (_instance == 0)
  {
    _instance = new CIOStatus;
	//Inicializar mouse buttons info
	_instance->current_mouse_left = _instance->current_mouse_right = _instance->current_mouse_middle = _instance->current_mouse_wheel = false;
  }
  return _instance;
}

CIOStatus::CIOStatus() 
	: prev_mouse_loc_is_valid( false ) {

	Player1 = new CXBOXController(1);
}

void CIOStatus::releaseMouse( ) {
	has_control_of_mouse = false;
	//::ShowCursor( TRUE );
}
void CIOStatus::adquireMouse( ) {
	has_control_of_mouse = true;
	prev_mouse_loc_is_valid = false;
	//::ShowCursor( FALSE );
}

// guardar el estado de los botones 'ahora'
void CIOStatus::update( float delta ) {

	// XInput (Mando XBOX 360)
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx

	//Variaci�n de posici�n del mouse
	prev_mouse_loc = curr_mouse_loc;
	::GetCursorPos( &curr_mouse_loc );
	if( prev_mouse_loc_is_valid ) {
		delta_mouse.x = curr_mouse_loc.x - prev_mouse_loc.x;
		delta_mouse.y = curr_mouse_loc.y - prev_mouse_loc.y;
	} else {
		delta_mouse.x = delta_mouse.y = 0;
		prev_mouse_loc_is_valid = true;
	}

	if( has_control_of_mouse ) {
		curr_mouse_loc.x = 640;
		curr_mouse_loc.y = 480;
		::SetCursorPos( curr_mouse_loc.x, curr_mouse_loc.y  );
	} else { 
		delta_mouse.x = delta_mouse.y = 0;
		prev_mouse_loc_is_valid = true;
	}

	XINPUT_GAMEPAD gamepad = Player1->GetState().Gamepad;

	// calcular el delta
	for( int i=0; i<BUTTONS_COUNT; ++i ) {
		TDigital &b = buttons[ i ];
		b.was_pressed = b.is_pressed;

		switch (i)
		{
		case CREATE_SHADOW:
			b.is_pressed = current_mouse_left || gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
			break;
		case SILENT_KILL:
			b.is_pressed = current_mouse_left || gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
			break;
		case AIM:
			b.is_pressed = current_mouse_right || gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
			break;
		case DECOY:
			b.is_pressed = isPressed('Q') || gamepad.wButtons & XINPUT_GAMEPAD_X;
			break;
		case RECHARGE:
			b.is_pressed = isPressed('R') || gamepad.wButtons & XINPUT_GAMEPAD_B;
			break;
		case SPECIAL_VISION:
			b.is_pressed = isPressed('E') || gamepad.wButtons & XINPUT_GAMEPAD_Y;
			break;
		case CROW:
			b.is_pressed = isPressed('C') /*|| gamepad.wButtons & XINPUT_GAMEPAD_Y*/; //crow de gamepad??
			break;
		case FIGHT:
			b.is_pressed = isPressed('F') || gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
			break;
		case TELEPORT:
			b.is_pressed = isPressed(32) || gamepad.wButtons & XINPUT_GAMEPAD_A; //32 == Space
			break;
		case BLEND:
			b.is_pressed = isPressed(32) || gamepad.wButtons & XINPUT_GAMEPAD_A; //32 == Space
			break;
		case FORWARD:
			b.is_pressed = isPressed('W');
			break;
		case BACKWARD:
			b.is_pressed = isPressed('S');
			break;
		case LEFT:
			b.is_pressed = isPressed('A');
			break;
		case RIGHT:
			b.is_pressed = isPressed('D');
			break;
		case SPRINT:
			b.is_pressed = isPressed(16);
			break;
		case KEYBOARD_ALT:
			b.is_pressed = isPressed(18);
			break;
		case KEYBOARD_Z:
			b.is_pressed = isPressed('Z');
			break;
		case KEYBOARD_F:
			b.is_pressed = isPressed('F');
			break;
		case KEYBOARD_T:
			b.is_pressed = isPressed('T');
			break;
		case MOUSE_MIDDLE:
			b.is_pressed = current_mouse_middle;
			break;
		case MOUSE_WHEEL_UP:
			b.is_pressed = current_mouse_wheel > 0;
			break;
		case MOUSE_WHEEL_DOWN:
			b.is_pressed = current_mouse_wheel < 0;
			break;
		case LEFT_STICK:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
			break;
		case RIGHT_STICK:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
			break;
		case D_UP:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
			break;
		case D_DOWN:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
			break;
		case D_LEFT:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
			break;
		case D_RIGHT:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
			break;
		case START:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_START;
			break;
		case BACK:
			b.is_pressed = gamepad.wButtons & XINPUT_GAMEPAD_BACK;
			break;
		case PRUEBAS:
			b.is_pressed = isPressed('F') || gamepad.wButtons & XINPUT_GAMEPAD_B;
			break;
		}

		// gestionado por los eventos WM_MOUSE*
		if( b.is_pressed )
			b.time_pressed += delta;
		if( b.becomesPressed( ))
			b.time_pressed = 0.0f;
	}

	// reset wheel status
	current_mouse_wheel = 0;


	if(Player1->IsConnected())
    {
		//XINPUT_GAMEPAD gamepad = Player1->GetState().Gamepad;

		float lx = gamepad.sThumbLX;
		float ly = gamepad.sThumbLY;

		/*dbg("lx: %u\n", lx);
		dbg("ly: %u\n", ly);*/

		//determine how far the controller is pushed
		float magnitude_l = sqrt(lx*lx + ly*ly);

		//determine the direction the controller is pushed
		float normalized_lx = lx / magnitude_l;
		float normalized_ly = ly / magnitude_l;

		if(normalized_lx != normalized_lx) normalized_lx = 0.0f;
		if(normalized_ly != normalized_ly) normalized_ly = 0.0f;

		/*dbg("normalized_lx: %f\n", normalized_lx);
		dbg("normalized_ly: %f\n", normalized_ly);*/

		float normalized_magnitude_l = 0.0f;

		//check if the controller is outside a circular dead zone
		if (magnitude_l > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
		  //clip the magnitude at its expected maximum value
		  if (magnitude_l > 32767) magnitude_l = 32767;
  
		  //adjust magnitude relative to the end of the dead zone
		  magnitude_l -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

		  //optionally normalize the magnitude with respect to its expected range
		  //giving a magnitude value of 0.0 to 1.0
		  normalized_magnitude_l = magnitude_l / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		}
		else //if the controller is in the deadzone zero out the magnitude
		{
			magnitude_l = 0.0f;
			normalized_magnitude_l = 0.0f;
		}


		float rx = gamepad.sThumbRX;
		float ry = gamepad.sThumbRY;

		/*dbg("lx: %u\n", lx);
		dbg("ly: %u\n", ly);*/

		//determine how far the controller is pushed
		float magnitude_r = sqrt(rx*rx + ry*ry);

		//determine the direction the controller is pushed
		float normalized_rx = rx / magnitude_r;
		float normalized_ry = ry / magnitude_r;

		if(normalized_rx != normalized_rx) normalized_rx = 0.0f;
		if(normalized_ry != normalized_ry) normalized_ry = 0.0f;

		/*dbg("normalized_rx: %f\n", normalized_rx);
		dbg("normalized_ry: %f\n", normalized_ry);
		*/

		float normalized_magnitude_r = 0.0f;

		//check if the controller is outside a circular dead zone
		if (magnitude_r > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
		  //clip the magnitude_r at its expected maximum value
		  if (magnitude_r > 32767) magnitude_r = 32767;
  
		  //adjust magnitude_r relative to the end of the dead zone
		  magnitude_r -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

		  //optionally normalize the magnitude_r with respect to its expected range
		  //giving a magnitude_r value of 0.0 to 1.0
		  normalized_magnitude_r = magnitude_r / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		}
		else //if the controller is in the deadzone zero out the magnitude_r
		{
			magnitude_r = 0.0f;
			normalized_magnitude_r = 0.0f;
		}
		

		//dbg("left trigger: %u\n", gamepad.bLeftTrigger);
		//dbg("right trigger: %u\n", gamepad.bRightTrigger);

		left.y = normalized_ly * normalized_magnitude_l;
		left.x = normalized_lx * normalized_magnitude_l;
		left.normalized_magnitude = normalized_magnitude_l;

		right.y = normalized_ry * normalized_magnitude_r;
		right.x = normalized_rx * normalized_magnitude_r;
		right.normalized_magnitude = normalized_magnitude_r;
		
		//dbg("right shoulder %i\n", isPressed( RIGHT_SHOULDER ));
	}
	else
	{
		left.x = 0;
		left.y = 0;
		left.normalized_magnitude = 0;
		right.x = 0;
		right.y = 0;
		right.normalized_magnitude = 0;
	}
}

bool CIOStatus::isPressed( TButton b ) const {
	return buttons[ b ].isPressed();
}
bool CIOStatus::becomesPressed( TButton b ) const {
	return buttons[ b ].becomesPressed();
}
bool CIOStatus::becomesReleased( TButton b ) const {
	return buttons[ b ].becomesReleased();
}
bool CIOStatus::isReleased( TButton b ) const {
	return buttons[ b ].isReleased();
}
bool CIOStatus::isPressed( int key ) const {
	return ( ::GetAsyncKeyState( key ) & 0x8000 ) != 0;
}
bool CIOStatus::becomesPressed_key( int key ) {
	bool isPressed = (::GetAsyncKeyState(key) & 0x8000)!= 0;
	if (isPressed == false) return false;

	if (currentKey != key || (clock()-currentKey_time_pressed > 250.0f)) 
	{
		currentKey = key;
		currentKey_time_pressed = clock();
		return true;
	}
	else return false;
}

void CIOStatus::vibrate( int left, int right )
{
	clamp(right, 0, 65535 );
	clamp(left, 0, 65535 );

	Player1->Vibrate(left, right);
}

float CIOStatus::getHorizontalAxis(void)
{
	float x = 0;
	if (isPressed(LEFT)) x = -1;
	else if (isPressed(RIGHT)) x = 1;
	else x = left.x;
				
	return x;
}

float CIOStatus::getVerticalAxis(void)
{
	float y = 0;
	if (isPressed(BACKWARD)) y = -1;
	else if (isPressed(FORWARD)) y = 1;
	else y = left.y;
	return y;
}

bool CIOStatus::theresMovement()
{
	return getHorizontalAxis() || getVerticalAxis();
}

float CIOStatus::getSpeed() const
{
	if( isPressed(SPRINT) )
		return 1.0f;
	else 
		return left.normalized_magnitude;// * left.normalized_magnitude * left.normalized_magnitude;
}
bool CIOStatus::isSprinting() const
{
	//dbg("stick %f\n", getSpeed());

	if( getSpeed() > 0.99f )
		return true;
	else 
		return false;
}
