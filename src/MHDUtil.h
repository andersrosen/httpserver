#pragma once

struct MHD_Daemon;
struct MHD_PostProcessor;

namespace ARo::Http::Internal {

    struct MhdDaemonDeleter {
        void operator()(struct MHD_Daemon* p) { MHD_stop_daemon(p); }
    };
    using MhdDaemon = std::unique_ptr<MHD_Daemon, MhdDaemonDeleter>;

    struct MhdPostProcessorDeleter {
        void operator()(struct MHD_PostProcessor *p) { MHD_destroy_post_processor(p); }
    };
    using MhdPostProcessor = std::unique_ptr<MHD_PostProcessor, MhdPostProcessorDeleter>;
}
