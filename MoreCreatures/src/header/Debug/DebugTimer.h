#ifndef DEBUG_TIMER_H
#define DEBUG_TIMER_H

#include <GLFW/glfw3.h>
#include <iostream>

// 단계별 시간 측정 디버그 헬퍼.
// 평소엔 enabled=false로 끄고, 진단 필요할 때만 true로 켬 — main 코드는 그대로 유지.
//
// 사용법:
//   DebugTimer t;             // 기본값 false → 출력 안 함 (평소 빌드)
//   DebugTimer t(true);       // 진단 모드 → report() 출력 켜짐
//
//   t.report("glfw + window");
//   t.report("셰이더 컴파일");
//   std::cout << "총 " << t.total() << " ms" << std::endl;
class DebugTimer
{
public:
    //enabled=false면 report()가 즉시 리턴 — 평소 빌드에 출력 노이즈 없음
    explicit DebugTimer(bool enabled = false)
        : enabled_(enabled), start_(glfwGetTime()), prev_(start_) {}

    //prev 시점부터 지금까지의 시간 출력 + prev 갱신.
    //enabled가 false면 출력 없이 prev만 갱신 (다음 report 측정에 영향 없도록).
    void report(const char* label)
    {
        double now = glfwGetTime();
        if (enabled_)
        {
            std::cout << "[debug] " << label << ": "
                      << (now - prev_) * 1000.0 << " ms"
                      << "  (누적 " << (now - start_) * 1000.0 << " ms)" << std::endl;
        }
        prev_ = now;
    }

    //전체 경과 시간 (시작부터 지금까지) — ms 단위. enabled 무관하게 항상 작동.
    double total() const { return (glfwGetTime() - start_) * 1000.0; }

    //런타임 토글 — 게임 중에 디버그 키로 켜고 끄는 경우용 (예: F2 누르면 측정 on)
    void setEnabled(bool e) { enabled_ = e; }
    bool isEnabled() const  { return enabled_; }

private:
    bool   enabled_;
    double start_;   //인스턴스 생성 시점 — total() 기준
    double prev_;    //마지막 report() 호출 시점 — 다음 report에서 구간 측정용
};

#endif
