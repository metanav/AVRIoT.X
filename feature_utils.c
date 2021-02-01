#include <stddef.h>
#include <math.h>
#include <stdint.h>

/** Root Mean Square */
float rms(int16_t *buf, size_t len)
{
    float sum = 0.0;
    for(size_t i = 0; i < len; i++) {
         float v = (float) (buf[i]/100.0f);
         sum += v * v;
    }
    return sqrt(sum / (float)len);
}

float skewness(int16_t *buf, size_t len)
{
    float sum = 0.0f;
    float mean;

    // Calculate the mean
    for (size_t i = 0; i < len; i++) {
        sum += (float) (buf[i]/100.0f);
    }
    mean = sum / len;

    // Calculate the m values
    float m_3 = 0.0f;
    float m_2 = 0.0f;

    for (size_t i = 0; i < len; i++) {
        float diff;
        diff = (float) (buf[i]/100.0f) - mean;
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

float std_dev(int16_t *buf, size_t len)
{
    float sum = 0.0f;

    for (size_t i = 0; i < len; i++) {
        sum += (float) (buf[i]/100.0f);
    }

    float mean = sum / len;

    float std = 0.0f;

    for (size_t i = 0; i < len; i++) {
        float diff;
        diff = (float) (buf[i]/100.0f) - mean;
        std += diff * diff;
    }

    return sqrt(std / len);
}

float kurtosis(int16_t *buf, size_t len)
{
    float mean = 0.0f;
    float sum = 0.0f;

    for (size_t i = 0; i < len; i++) {
        sum += (float) (buf[i]/100.0f);
    }
    mean = sum / len;

    // Calculate m_4 & variance
    float m_4 = 0.0f;
    float variance = 0.0f;

    for (size_t i = 0; i < len; i++) {
        float diff;
        diff =(float) (buf[i]/100.0f) - mean;
        float square_diff = diff * diff;
        variance += square_diff;
        m_4 += square_diff * square_diff;
    }
    m_4 = m_4 / len;
    variance = variance / len;

    // Calculate Fisher kurtosis = (m_4 / variance^2) - 3
    return (m_4 / (variance * variance)) - 3;
}
