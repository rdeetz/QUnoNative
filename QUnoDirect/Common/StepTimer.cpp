// QUnoDirect - StepTimer.cpp
// 2021 Roger Deetz

#include "pch.h"
#include "StepTimer.h"

using namespace DX;

StepTimer::StepTimer() noexcept(false) : 
    _elapsedTicks(0),
    _totalTicks(0),
    _leftOverTicks(0),
    _frameCount(0),
    _framesPerSecond(0),
    _framesThisSecond(0),
    _qpcSecondCounter(0),
    _isFixedTimeStep(false),
    _targetElapsedTicks(TicksPerSecond / 60)

{
    if (!QueryPerformanceFrequency(&_qpcFrequency))
    {
        throw std::exception();
    }

    if (!QueryPerformanceCounter(&_qpcLastTime))
    {
        throw std::exception();
    }

    // Initialize max delta to 1/10 of a second.
    _qpcMaxDelta = static_cast<uint64_t>(_qpcFrequency.QuadPart / 10);

    return;
}

/*
template<typename TUpdate> void StepTimer::Tick(const TUpdate& update)
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
*/

void StepTimer::ResetElapsedTime()
{
    if (!QueryPerformanceCounter(&_qpcLastTime))
    {
        throw std::exception();
    }

    _leftOverTicks = 0;
    _framesPerSecond = 0;
    _framesThisSecond = 0;
    _qpcSecondCounter = 0;

    return;
}
