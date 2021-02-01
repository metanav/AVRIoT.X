#include <stddef.h>
#include <math.h>

/** Root Mean Square */
float rms(float *buf, size_t len)
{
    float sum = 0.0;
    for(size_t i = 0; i < len; i++) {
         float v = buf[i];
         sum += v * v;
    }
    return sqrt(sum / (float)len);
}

float skewness(float *buf, size_t len)
{
    float sum = 0.0f;
    float mean;

    // Calculate the mean
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    mean = sum / len;

    // Calculate the m values
    float m_3 = 0.0f;
    float m_2 = 0.0f;

    for (size_t i = 0; i < len; i++) {
        float diff;
        diff = buf[i] - mean;
        m_3 += diff * diff * diff;
        m_2 += diff * diff;
    }
    m_3 = m_3 / len;
    m_2 = m_2 / len;

    // Calculate (m_2)^(3/2)
    m_2 = sqrt(m_2 * m_2 * m_2);

    // Calculate skew = (m_3) / (m_2)^(3/2)
    return m_3 / m_2;
}

float std_dev(float *buf, size_t len)
{
    float sum = 0.0f;

    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }

    float mean = sum / len;

    float std = 0.0f;

    for (size_t i = 0; i < len; i++) {
        float diff;
        diff = buf[i] - mean;
        std += diff * diff;
    }

    return sqrt(std / len);
}

float kurtosis(float *buf, size_t len)
{
    float mean = 0.0f;
    float sum = 0.0f;

    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    mean = sum / len;

    // Calculate m_4 & variance
    float m_4 = 0.0f;
    float variance = 0.0f;

    for (size_t i = 0; i < len; i++) {
        float diff;
        diff = buf[i] - mean;
        float square_diff = diff * diff;
        variance += square_diff;
        m_4 += square_diff * square_diff;
    }
    m_4 = m_4 / len;
    variance = variance / len;

    // Calculate Fisher kurtosis = (m_4 / variance^2) - 3
    return (m_4 / (variance * variance)) - 3;
}
