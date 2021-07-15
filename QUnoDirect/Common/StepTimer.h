// QUnoDirect - StepTimer.h
// 2021 Roger Deetz

#pragma once

#include <cmath>
#include <cstdint>
#include <exception>

namespace DX
{
    class StepTimer
    {
    public:
        StepTimer() noexcept(false);

        uint64_t GetElapsedTicks() const noexcept { return _elapsedTicks; }
        double GetElapsedSeconds() const noexcept { return TicksToSeconds(_elapsedTicks); }
        uint64_t GetTotalTicks() const noexcept { return _totalTicks; }
        double GetTotalSeconds() const noexcept { return TicksToSeconds(_totalTicks); }
        uint32_t GetFrameCount() const noexcept { return _frameCount; }
        uint32_t GetFramesPerSecond() const noexcept { return _framesPerSecond; }
        void SetFixedTimeStep(bool isFixedTimestep) noexcept { _isFixedTimeStep = isFixedTimestep; return;  }
        void SetTargetElapsedTicks(uint64_t targetElapsed) noexcept { _targetElapsedTicks = targetElapsed; return;  }
        void SetTargetElapsedSeconds(double targetElapsed) noexcept { _targetElapsedTicks = SecondsToTicks(targetElapsed); return;  }

        // Integer format represents time using 10,000,000 ticks per second.
        static const uint64_t TicksPerSecond = 10000000;

        static constexpr double TicksToSeconds(uint64_t ticks) noexcept { return static_cast<double>(ticks) / TicksPerSecond; }
        static constexpr uint64_t SecondsToTicks(double seconds) noexcept { return static_cast<uint64_t>(seconds * TicksPerSecond); }

        template<typename TUpdate> void Tick(const TUpdate& update)
        {
            LARGE_INTEGER currentTime;

            if (!QueryPerformanceCounter(&currentTime))
            {
                throw std::exception();
            }

            uint64_t timeDelta = static_cast<uint64_t>(currentTime.QuadPart - _qpcLastTime.QuadPart);
            _qpcLastTime = currentTime;
            _qpcSecondCounter += timeDelta;

            // Clamp excessively large time deltas (e.g. after paused in the debugger).
            if (timeDelta > _qpcMaxDelta)
            {
                timeDelta = _qpcMaxDelta;
            }

            // Convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
            timeDelta *= TicksPerSecond;
            timeDelta /= static_cast<uint64_t>(_qpcFrequency.QuadPart);

            uint32_t lastFrameCount = _frameCount;

            if (_isFixedTimeStep)
            {
                // If the app is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
                // the clock to exactly match the target value. This prevents tiny and irrelevant errors
                // from accumulating over time. Without this clamping, a game that requested a 60 fps
                // fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
                // accumulate enough tiny errors that it would drop a frame. It is better to just round
                // small deviations down to zero to leave things running smoothly.
                if (static_cast<uint64_t>(std::abs(static_cast<int64_t>(timeDelta - _targetElapsedTicks))) < TicksPerSecond / 4000)
                {
                    timeDelta = _targetElapsedTicks;
                }

                _leftOverTicks += timeDelta;

                while (_leftOverTicks >= _targetElapsedTicks)
                {
                    _elapsedTicks = _targetElapsedTicks;
                    _totalTicks += _targetElapsedTicks;
                    _leftOverTicks -= _targetElapsedTicks;
                    _frameCount++;

                    update();
                }
            }
            else
            {
                _elapsedTicks = timeDelta;
                _totalTicks += timeDelta;
                _leftOverTicks = 0;
                _frameCount++;

                update();
            }

            // Track the current framerate.
            if (_frameCount != lastFrameCount)
            {
                _framesThisSecond++;
            }

            if (_qpcSecondCounter >= static_cast<uint64_t>(_qpcFrequency.QuadPart))
            {
                _framesPerSecond = _framesThisSecond;
                _framesThisSecond = 0;
                _qpcSecondCounter %= static_cast<uint64_t>(_qpcFrequency.QuadPart);
            }

            return;
        }

        // After an intentional timing discontinuity (for instance a blocking IO operation)
        // call this to avoid having the fixed timestep logic attempt a set of catch-up Update calls.
        void ResetElapsedTime();

    private:
        // Source timing data uses QPC units.
        LARGE_INTEGER _qpcFrequency;
        LARGE_INTEGER _qpcLastTime;
        uint64_t _qpcMaxDelta;

        // Derived timing data uses a canonical tick format.
        uint64_t _elapsedTicks;
        uint64_t _totalTicks;
        uint64_t _leftOverTicks;

        // Members for tracking the framerate.
        uint32_t _frameCount;
        uint32_t _framesPerSecond;
        uint32_t _framesThisSecond;
        uint64_t _qpcSecondCounter;

        // Members for configuring fixed timestep mode.
        bool _isFixedTimeStep;
        uint64_t _targetElapsedTicks;
    };
}
