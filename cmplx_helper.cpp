#include <Arduino.h>
#include "cmplx_helper.h"


cmplx_phasor_t cmplxGetPhasor(cmplx_versor_t v) {
  cmplx_phasor_t phasor;
  phasor.x = v.z * cos(v.phi);
  phasor.jy = v.z * sin(v.phi);
  return phasor;
}

cmplx_versor_t cmplxGetVersor(cmplx_phasor_t p) {
  cmplx_versor_t versor;
  versor.z = sqrtf(p.x * p.x + p.jy * p.jy);
  versor.phi = atan(p.x / p.jy);
  return versor;
}

cmplx_phasor_t cmplxAddPhasor(cmplx_phasor_t p1, cmplx_phasor_t p2) {
  cmplx_phasor_t phasor;
  phasor.x = p1.x + p2.x;
  phasor.jy = p1.jy + p2.jy;
  return phasor;
}

cmplx_phasor_t cmplxSubstractPhasor(cmplx_phasor_t p1, cmplx_phasor_t p2) {
  cmplx_phasor_t phasor;
  phasor.x = p1.x - p2.x;
  phasor.jy = p1.jy - p2.jy;
  return phasor;
}

cmplx_versor_t cmplxSubstractVersor(cmplx_versor_t p1, cmplx_versor_t p2) {
  return cmplxGetVersor(cmplxSubstractPhasor(cmplxGetPhasor(p1), cmplxGetPhasor(p2)));
}

cmplx_versor_t cmplxMultiplyVersor(cmplx_versor_t v1, cmplx_versor_t v2) {
  cmplx_versor_t versor;
  versor.z = v1.z * v2.z;
  versor.phi = v1.phi + v2.phi;
  return versor;
}

cmplx_phasor_t cmplxMultiplyPhasor(cmplx_phasor_t p1, cmplx_phasor_t p2) {
  return cmplxGetPhasor(cmplxMultiplyVersor(cmplxGetVersor(p1), cmplxGetVersor(p2)));
}

cmplx_versor_t cmplxDevideVersor(cmplx_versor_t v1, cmplx_versor_t v2) {
  cmplx_versor_t versor;
  versor.z = v1.z / v2.z;
  versor.phi = v1.phi - v2.phi;
  return versor;
}
