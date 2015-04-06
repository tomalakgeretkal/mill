set_global("std::io::writeln", make_subroutine<string>([&thread_pool, &io_service] (string const& mill_string) {
    boost::asio::posix::stream_descriptor out(io_service, ::dup(STDOUT_FILENO));
    auto string = mill_string.data() + '\n';

    auto& current_fiber = fiber::current();
    boost::asio::async_write(
        out,
        boost::asio::buffer(string.data(), string.size()),
        [&] (boost::system::error_code ec, std::size_t bytes_transfered) {
            (void)ec; // TODO: Handle error!
            (void)bytes_transfered; // TODO: Retry!
            thread_pool.resume(current_fiber);
        }
    );

    fiber::pause();

    return handle(unit());
}));
