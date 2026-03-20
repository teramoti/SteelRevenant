//------------------------//------------------------
// Contents(処理内容) 高精度タイマによるフレーム時間管理クラスを宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include<windows.h>
#include <stdexcept>
#include <exception>
#include <stdint.h>

namespace DX
{
    // アニメーションとシミュレーションの時間計測を管理する。
    class StepTimer
    {
    public:
        // QPC ベースの計測状態を初期化する。
        StepTimer() : 
            m_elapsedTicks(0),
            m_totalTicks(0),
            m_leftOverTicks(0),
            m_frameCount(0),
            m_framesPerSecond(0),
            m_framesThisSecond(0),
            m_qpcSecondCounter(0),
            m_isFixedTimeStep(false),
            m_targetElapsedTicks(TicksPerSecond / 60)
        {
            if (!QueryPerformanceFrequency(&m_qpcFrequency))
            {
                throw std::exception( "QueryPerformanceFrequency" );
            }

            if (!QueryPerformanceCounter(&m_qpcLastTime))
            {
                throw std::exception( "QueryPerformanceCounter" );
            }

            // 長時間停止後の急激な時間差を抑える上限値を設定する。
            m_qpcMaxDelta = m_qpcFrequency.QuadPart / 10;
        }

        // 前回更新からの経過ティック数を返す。
        uint64_t GetElapsedTicks() const					{ return m_elapsedTicks; }
        // 前回更新からの経過秒数を返す。
        double GetElapsedSeconds() const					{ return TicksToSeconds(m_elapsedTicks); }

        // 開始からの累積ティック数を返す。
        uint64_t GetTotalTicks() const						{ return m_totalTicks; }
        // 開始からの累積秒数を返す。
        double GetTotalSeconds() const						{ return TicksToSeconds(m_totalTicks); }

        // 開始からの更新回数を返す。
        uint32_t GetFrameCount() const						{ return m_frameCount; }

        // 直近 1 秒のフレーム数を返す。
        uint32_t GetFramesPerSecond() const					{ return m_framesPerSecond; }

        // 固定タイムステップを使用するか設定する。
        void SetFixedTimeStep(bool isFixedTimestep)			{ m_isFixedTimeStep = isFixedTimestep; }

        // 固定タイムステップ時の更新間隔をティック単位で設定する。
        void SetTargetElapsedTicks(uint64_t targetElapsed)	{ m_targetElapsedTicks = targetElapsed; }
        // 固定タイムステップ時の更新間隔を秒単位で設定する。
        void SetTargetElapsedSeconds(double targetElapsed)	{ m_targetElapsedTicks = SecondsToTicks(targetElapsed); }

        // 内部時間は 1 秒 = 10,000,000 ティックで表現する。
        static const uint64_t TicksPerSecond = 10000000;

        // ティック数を秒へ変換する。
        static double TicksToSeconds(uint64_t ticks)		{ return static_cast<double>(ticks) / TicksPerSecond; }
        // 秒数をティックへ変換する。
        static uint64_t SecondsToTicks(double seconds)		{ return static_cast<uint64_t>(seconds * TicksPerSecond); }

        // 経過時間をリセットし、復帰直後の追いつき更新を防ぐ。
        void ResetElapsedTime()
        {
            if (!QueryPerformanceCounter(&m_qpcLastTime))
            {
                throw std::exception("QueryPerformanceCounter");
            }

            m_leftOverTicks = 0;
            m_framesPerSecond = 0;
            m_framesThisSecond = 0;
            m_qpcSecondCounter = 0;
        }

        // タイマ状態を更新し、必要回数だけ更新関数を呼ぶ。
        template<typename TUpdate>
        void Tick(const TUpdate& update)
        {
            // 現在時刻を取得する。
            LARGE_INTEGER currentTime;

            if (!QueryPerformanceCounter(&currentTime))
            {
                throw std::exception( "QueryPerformanceCounter" );
            }

            uint64_t timeDelta = currentTime.QuadPart - m_qpcLastTime.QuadPart;

            m_qpcLastTime = currentTime;
            m_qpcSecondCounter += timeDelta;

            // 大きすぎる時間差を上限値へ丸める。
            if (timeDelta > m_qpcMaxDelta)
            {
                timeDelta = m_qpcMaxDelta;
            }

            // QPC 単位を内部ティック表現へ変換する。
            timeDelta *= TicksPerSecond;
            timeDelta /= m_qpcFrequency.QuadPart;

            uint32_t lastFrameCount = m_frameCount;

            if (m_isFixedTimeStep)
            {
                // 目標時間との差が小さい場合は目標値へ丸める。
                if (abs(static_cast<int64_t>(timeDelta - m_targetElapsedTicks)) < TicksPerSecond / 4000)
                {
                    timeDelta = m_targetElapsedTicks;
                }

                m_leftOverTicks += timeDelta;

                while (m_leftOverTicks >= m_targetElapsedTicks)
                {
                    m_elapsedTicks = m_targetElapsedTicks;
                    m_totalTicks += m_targetElapsedTicks;
                    m_leftOverTicks -= m_targetElapsedTicks;
                    m_frameCount++;

                    update();
                }
            }
            else
            {
                // 可変タイムステップでは 1 回だけ更新する。
                m_elapsedTicks = timeDelta;
                m_totalTicks += timeDelta;
                m_leftOverTicks = 0;
                m_frameCount++;

                update();
            }

            // 直近 1 秒のフレーム数を集計する。
            if (m_frameCount != lastFrameCount)
            {
                m_framesThisSecond++;
            }

            if (m_qpcSecondCounter >= static_cast<uint64_t>(m_qpcFrequency.QuadPart))
            {
                m_framesPerSecond = m_framesThisSecond;
                m_framesThisSecond = 0;
                m_qpcSecondCounter %= m_qpcFrequency.QuadPart;
            }
        }

    private:
        // QPC 由来の時刻情報。
        LARGE_INTEGER m_qpcFrequency;
        LARGE_INTEGER m_qpcLastTime;
        uint64_t m_qpcMaxDelta;

        // 内部ティック表現の時刻情報。
        uint64_t m_elapsedTicks;
        uint64_t m_totalTicks;
        uint64_t m_leftOverTicks;

        // フレームレート集計用の値。
        uint32_t m_frameCount;
        uint32_t m_framesPerSecond;
        uint32_t m_framesThisSecond;
        uint64_t m_qpcSecondCounter;

        // 固定タイムステップ設定。
        bool m_isFixedTimeStep;
        uint64_t m_targetElapsedTicks;
    };
}

