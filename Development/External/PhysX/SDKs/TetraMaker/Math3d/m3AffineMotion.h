#ifndef M3_AFFINE_MOTION_H
#define M3_AFFINE_MOTION_H

//---------------------------------------------------------------------------

#include "math3d.h"

//---------------------------------------------------------------------------
class m3AffineMotion
//---------------------------------------------------------------------------
{	
public:
	m3AffineMotion() { pose0.id(); pose1.id(); a.zero(); b.zero(); c.zero(); d.zero(); }
	void set(const m3Affine &fromPose, const m3Affine &toPose, int primaryAxis = 0, bool forward = true) {
		axisNr = primaryAxis;
		axisForward = forward;
		pose0 = fromPose; pose1 = toPose;
		pose0.orthoNormalize(); pose1.orthoNormalize();
		m3Real len = 2.0f * pose0.pos.distance(pose1.pos);
		m3Vector dir0 = pose0.mat.getColumn(axisNr) * len;
		m3Vector dir1 = pose1.mat.getColumn(axisNr) * len;
		if (!axisForward) { dir0 = -dir0; dir1 = -dir1; }
		d = pose0.pos;
		c = dir0;
		a = (pose0.pos - pose1.pos)*2.0f + dir1 + dir0;
		b = pose1.pos - dir0 - a - pose0.pos;
		m3Matrix rot; rot.multiplyTransposedRight(pose1.mat, pose0.mat);
		rot.getAxisAndAngle(axis, angle);
	}
	void interpolate(m3Real t, m3Affine &pose, bool slowDown) {
		t = m3Clamp(t, 0.0f, 1.0f);
		if (slowDown)
			t = (-2.0f*t + 3.0f)*t*t;	// smooth start, smooth end
		pose.pos = ((a*t + b)*t + c)*t + d;
		m3Vector dir0 = (a*t*3.0f + b*2.0f)*t + c; dir0.normalize();
		m3Matrix mat; mat.rot(axis, angle*t);
		mat = mat * pose0.mat;
		m3Vector dir = mat.getColumn(axisNr);
		if (!axisForward) dir = -dir;
		m3Matrix rot; rot.rot(dir, dir0);
		pose.mat = rot * mat;
	}
private:
	m3Affine pose0;
	m3Affine pose1;
	m3Vector a,b,c,d;	// spline coefficients
	m3Vector axis;		// frame spherical interpolation
	m3Real   angle;
	int axisNr;
	bool axisForward;
};


#endif
