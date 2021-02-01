/* 
 * File:   feature_utils.h
 * Author: naveen
 *
 * Created on January 29, 2021, 5:57 PM
 */

#ifndef FEATURE_UTILS_H
#define	FEATURE_UTILS_H

#ifdef	__cplusplus
extern "C" {
#endif

float rms(float *buf, size_t len);

float skewness(float *buf, size_t len);

float std_dev(float *buf, size_t len);

float kurtosis(float *buf, size_t len);

#ifdef	__cplusplus
}
#endif

#endif	/* FEATURE_UTILS_H */

