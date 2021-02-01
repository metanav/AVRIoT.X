/**
* Compute dot product
*/
static float dot(float *x, ...) {
    va_list w;
    va_start(w, 12);
    float dot = 0.0;

    for (uint16_t i = 0; i < 12; i++) {
        const float wi = va_arg(w, double);
        dot += x[i] * wi;
    }

    return dot;
}

/**
* Predict class for features vector
*/
int predict(float *x) {
    return dot(x,   -0.47740808  , -0.56071025  , 0.018827347  , -0.653598  , 0.043264825  , -0.34762117  , 0.0070606587  , -0.5321579  , -0.19677076  , -0.41591045  , 0.0068177436  , -0.484328 ) <= -0.6410006758482234 ? 0 : 1;
}
