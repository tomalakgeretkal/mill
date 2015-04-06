set_global("std::conc::sleep", make_subroutine([&thread_pool, &io_service] {
    boost::asio::deadline_timer timer(io_service);
    timer.expires_from_now(boost::posix_time::milliseconds(1000));

    auto& current_fiber = fiber::current();
    timer.async_wait([&] (boost::system::error_code ec) {
        (void)ec; // TODO: Handle error!
        thread_pool.resume(current_fiber);
    });
    fiber::pause();

    return handle(unit());
}));

set_global("std::conc::spawn", make_subroutine<subroutine>([&thread_pool] (subroutine const& entry) {
    auto& fiber = thread_pool.spawn([entry = std::move(entry)] () mutable {
        std::vector<handle> arguments;
        entry(arguments.begin(), arguments.end());
    });
    thread_pool.resume(fiber);
    return handle(unit());
}));
