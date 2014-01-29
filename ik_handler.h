#ifndef INC_IK_HANDLER_H_
#define INC_IK_HANDLER_H_

#include <d3dx9.h>

struct TIKHandler {

	// input params
	float ab;
	float bc;

	// working values
	float ac;
	float dist_parallel;
	float dist_perp;

	enum TState {
		NORMAL
	,   TOO_FAR
	,   TOO_CLOSE
	};

	TState state;

	//
	D3DXVECTOR3 a,b,c;
	D3DXVECTOR3 normal_abc;

	void init( D3DXVECTOR3 new_a, D3DXVECTOR3 new_b, D3DXVECTOR3 new_c ) {

		a = new_a;
		b = new_b;
		c = new_c;

		D3DXVECTOR3 dir_ab = b - a;
		D3DXVECTOR3 dir_bc = c - b;

		ab = D3DXVec3Length( &dir_ab );
		bc = D3DXVec3Length( &dir_bc );

		D3DXVec3Cross( &normal_abc, &dir_bc, &dir_ab );
		D3DXVec3Normalize( &normal_abc, &normal_abc );

		state = NORMAL;
	}

	bool updateB( ){

		D3DXVECTOR3 dir_ac = c - a;

		ac = D3DXVec3Length( &dir_ac );

		if( ac == 0.0f ) {
			state = TOO_CLOSE;
			return false;
		}
		if( ac > ab + bc ) {
			state = TOO_FAR;
			return false;
		}
		state = NORMAL;

		float num = ab*ab + ac*ac - bc*bc;
		float den = 2 * ac;

		dist_parallel = num / den;
		
		// d = dist_parallel
		// h = dist_perp
		// ab^2 = d^2 + h^2
		float dist_perp_sqr = ab*ab - dist_parallel*dist_parallel;
		assert( dist_perp_sqr >= 0.0f );
		dist_perp = sqrtf( dist_perp_sqr );

		// ab = d_para * ac  + d_perp * h
		// d_perp * h = ab - d_para * ac
		D3DXVECTOR3 para_dir = c - a;
		D3DXVec3Normalize( &para_dir, &para_dir );
		D3DXVECTOR3 perp_dir;
		D3DXVec3Cross( &perp_dir, &normal_abc, &para_dir );
		D3DXVec3Normalize( &perp_dir, &perp_dir );

		b = a + para_dir * dist_parallel + dist_perp * perp_dir;

		return true;
	}

};


#endif
