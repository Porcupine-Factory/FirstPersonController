#pragma once

#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector3.h>

namespace FirstPersonController
{
    // Templated PID controller for scalar or vector control
    // Supports two derivative modes: ErrorRate (dError/dt) for standard PID, Velocity (-dValue/dt) for velocity feedback
    // Includes anti-windup on integral and low-pass filtering on derivative for stability
    template<class T>
    class PidController
    {
    public:
        enum DerivativeCalculationMode
        {
            ErrorRate,
            Velocity
        };

        // Default constructor with zero gains and defaults
        PidController()
            : PidController(0.0f, 0.0f, 0.0f, 100.0f, 0.8f, ErrorRate)
        {
        }

        // Parameterized constructor for gains and settings
        PidController(
            float proportionalGain,
            float integralGain,
            float derivativeGain,
            float integralWindupLimit = 100.0f,
            float derivativeFilterAlpha = 0.8f,
            DerivativeCalculationMode mode = ErrorRate)
            : m_proportionalGain(proportionalGain)
            , m_integralGain(integralGain)
            , m_derivativeGain(derivativeGain)
            , m_integralWindupLimit(integralWindupLimit)
            , m_derivativeFilterAlpha(derivativeFilterAlpha)
            , m_derivativeMode(mode)
        {
            Reset();
        }

        DerivativeCalculationMode GetMode() const
        {
            return m_derivativeMode;
        }

        void SetDerivativeMode(DerivativeCalculationMode mode)
        {
            m_derivativeMode = mode;
        }

        void SetProportionalGain(const float proportionalGain)
        {
            m_proportionalGain = proportionalGain;
        }

        void SetIntegralGain(const float integralGain)
        {
            m_integralGain = integralGain;
        }

        void SetDerivativeGain(const float derivativeGain)
        {
            m_derivativeGain = derivativeGain;
        }

        void SetIntegralWindupLimit(const float integralWindupLimit)
        {
            m_integralWindupLimit = integralWindupLimit;
        }

        void SetDerivativeFilterAlpha(const float derivativeFilterAlpha)
        {
            m_derivativeFilterAlpha = derivativeFilterAlpha;
        }

        // Computes PID output based on current error and timestep.
        T Output(const T& error, float deltaTime, const T& currentValue)
        {
            if (!m_isInitialized)
            {
                // Initialize on first call to skip derivative jump
                m_isInitialized = true;
                m_previousError = error;
                m_previousValue = currentValue;
                m_integralAccumulator = GetZeroValue();
                m_previousDerivative = GetZeroValue();
                return error * m_proportionalGain + m_integralAccumulator; // Skip derivative on first call
            }

            // Proportional term: direct response to error
            T proportional = error * m_proportionalGain;

            // Integral term: accumulate error over time with anti-windup clamping
            m_integralAccumulator += error * deltaTime * m_integralGain;
            m_integralAccumulator = Clamp(m_integralAccumulator, -m_integralWindupLimit, m_integralWindupLimit);

            // Derivative term: rate of change
            // Helps prevent derivative kick caused by changes in error.
            // Useful for velocity-based applications where target values often change.
            T rawDerivative;
            if (m_derivativeMode == Velocity)
            {
                // Use negative velocity for damping
                T value_rate = (currentValue - m_previousValue) / deltaTime;
                rawDerivative = -value_rate;
            }
            else
            {
                // Standard error rate
                rawDerivative = (error - m_previousError) / deltaTime;
            }

            /// Apply low-pass filter to derivative for noise reduction (equivalent to lerp towards raw)
            T derivative = m_previousDerivative + m_derivativeFilterAlpha * ((rawDerivative * m_derivativeGain) - m_previousDerivative);
            m_previousDerivative = derivative;

            // Update previous values
            m_previousError = error;
            m_previousValue = currentValue;

            return proportional + m_integralAccumulator + derivative;
        }

        // Resets internal state (integral, previous values)
        void Reset()
        {
            m_integralAccumulator = GetZeroValue();
            m_previousError = GetZeroValue();
            m_previousDerivative = GetZeroValue();
            m_previousValue = GetZeroValue();
            m_isInitialized = false;
        }

        T GetLastProportional() const
        {
            return m_previousError * m_proportionalGain;
        }

        T GetLastIntegral() const
        {
            return m_integralAccumulator;
        }

        T GetLastDerivative() const
        {
            return m_previousDerivative;
        }

    private:
        float m_proportionalGain;
        float m_integralGain;
        float m_derivativeGain;
        float m_integralWindupLimit;
        float m_derivativeFilterAlpha;
        DerivativeCalculationMode m_derivativeMode;

        T m_integralAccumulator;
        T m_previousError;
        T m_previousDerivative;
        T m_previousValue;
        bool m_isInitialized;

        // Helper: Returns zero-initialized T (specialized for types)
        static T GetZeroValue()
        {
            return T(0.0f);
        }

        // Helper: Clamps value between min/max (vector/scalar via operators)
        static T Clamp(const T& val, float min, float max)
        {
            return val.GetClamped(T(min), T(max));
        }
    };

    // Specialize Zero/Clamp for Vector2
    template<>
    inline AZ::Vector2 PidController<AZ::Vector2>::GetZeroValue()
    {
        return AZ::Vector2::CreateZero();
    }

    template<>
    inline AZ::Vector2 PidController<AZ::Vector2>::Clamp(const AZ::Vector2& val, float min, float max)
    {
        return AZ::Vector2(AZ::GetClamp(val.GetX(), min, max), AZ::GetClamp(val.GetY(), min, max));
    }

    // Specialize Zero/Clamp for Vector3
    template<>
    inline AZ::Vector3 PidController<AZ::Vector3>::GetZeroValue()
    {
        return AZ::Vector3::CreateZero();
    }

    template<>
    inline AZ::Vector3 PidController<AZ::Vector3>::Clamp(const AZ::Vector3& val, float min, float max)
    {
        return AZ::Vector3(AZ::GetClamp(val.GetX(), min, max), AZ::GetClamp(val.GetY(), min, max), AZ::GetClamp(val.GetZ(), min, max));
    }

    // For float (scalar)
    template<>
    inline float PidController<float>::GetZeroValue()
    {
        return 0.0f;
    }

    template<>
    inline float PidController<float>::Clamp(const float& val, float min, float max)
    {
        return AZ::GetClamp(val, min, max);
    }
}; // namespace FirstPersonController