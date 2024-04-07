#ifndef cmplx_helper_h
#define cmplx_helper_h


#include "Arduino.h"

typedef struct cmplx_phasor_struct {
  float x;
  float jy;
} cmplx_phasor_t;

typedef struct cmplx_versor_struct {
  float z;
  float phi;
} cmplx_versor_t;

/*
 * Get a phasor form a versor.
 */
cmplx_phasor_t cmplxGetPhasor(cmplx_versor_t v);

/*
 * Get a versor form a phasor.
 */
cmplx_versor_t cmplxGetVersor(cmplx_phasor_t p);

/*
 * Add two phasors.
 */
cmplx_phasor_t cmplxAddPhasor(cmplx_phasor_t p1, cmplx_phasor_t p2);

/*
 * Substract two versors.
 */
cmplx_versor_t cmplxSubstractVersor(cmplx_versor_t p1, cmplx_versor_t p2);

/*
 * Substract two phasors.
 */
cmplx_phasor_t cmplxSubstractPhasor(cmplx_phasor_t p1, cmplx_phasor_t p2);

/*
 * Multiply two versors.
 */
cmplx_versor_t cmplxMultiplyVersor(cmplx_versor_t v1, cmplx_versor_t v2);

/*
 * Multiply two phasors.
 */
cmplx_phasor_t cmplxMultiplyPhasor(cmplx_phasor_t p1, cmplx_phasor_t p2);

/*
 * Devide two versors.
 */
cmplx_versor_t cmplxDevideVersor(cmplx_versor_t v1, cmplx_versor_t v2);

#endif