#include "SystemError.h"
#include <system_error>
#include <cstddef>
#include <Windows.h>

#include "Assertion.h"
#include "Error.h"
#include "Macro.h"
#include "StringEncoder.h"
#include "Typedef.h"
#include "StringFormat.h"

namespace jug
{

StringView ErrnoErrorCategory::GetName() const noexcept
{
    return "Errno";
}

String ErrnoErrorCategory::MakeErrorMessage(
    const int _err) const
{
    switch (static_cast<std::errc>(_err))
    {
        case std::errc::address_family_not_supported:
            return "Address family not supported by protocol";
        case std::errc::address_in_use:
            return "Address already in use";
        case std::errc::address_not_available:
            return "Address not available";
        case std::errc::already_connected:
            return "Socket is already connected";
        case std::errc::argument_list_too_long:
            return "Argument list too long";
        case std::errc::argument_out_of_domain:
            return "Numerical argument out of domain";
        case std::errc::bad_address:
            return "Bad address";
        case std::errc::bad_file_descriptor:
            return "Bad file descriptor";
        case std::errc::bad_message:
            return "Bad message";
        case std::errc::broken_pipe:
            return "Broken pipe";
        case std::errc::connection_aborted:
            return "Connection aborted";
        case std::errc::connection_already_in_progress:
            return "Connection already in progress";
        case std::errc::connection_refused:
            return "Connection refused";
        case std::errc::connection_reset:
            return "Connection reset";
        case std::errc::cross_device_link:
            return "Cross-device link";
        case std::errc::destination_address_required:
            return "Destination address required";
        case std::errc::device_or_resource_busy:
            return "Device or resource busy";
        case std::errc::directory_not_empty:
            return "Directory not empty";
        case std::errc::executable_format_error:
            return "Executable format error";
        case std::errc::file_exists:
            return "File exists";
        case std::errc::file_too_large:
            return "File too large";
        case std::errc::filename_too_long:
            return "Filename too long";
        case std::errc::function_not_supported:
            return "Function not supported";
        case std::errc::host_unreachable:
            return "Host unreachable";
        case std::errc::identifier_removed:
            return "Identifier removed";
        case std::errc::illegal_byte_sequence:
            return "Illegal byte sequence";
        case std::errc::inappropriate_io_control_operation:
            return "Inappropriate I/O control operation";
        case std::errc::interrupted:
            return "Interrupted function call";
        case std::errc::invalid_argument:
            return "Invalid argument";
        case std::errc::invalid_seek:
            return "Invalid seek";
        case std::errc::io_error:
            return "I/O error";
        case std::errc::is_a_directory:
            return "Is a directory";
        case std::errc::message_size:
            return "Message too long";
        case std::errc::network_down:
            return "Network is down";
        case std::errc::network_reset:
            return "Connection aborted by network";
        case std::errc::network_unreachable:
            return "Network unreachable";
        case std::errc::no_buffer_space:
            return "No buffer space available";
        case std::errc::no_child_process:
            return "No child processes";
        case std::errc::no_link:
            return "Link has been severed";
        case std::errc::no_lock_available:
            return "No locks available";
        case std::errc::no_message_available:
            return "No message available";
        case std::errc::no_message:
            return "No message of desired type";
        case std::errc::no_protocol_option:
            return "Protocol not available";
        case std::errc::no_space_on_device:
            return "No space left on device";
        case std::errc::no_stream_resources:
            return "No STREAM resources";
        case std::errc::no_such_device_or_address:
            return "No such device or address";
        case std::errc::no_such_device:
            return "No such device";
        case std::errc::no_such_file_or_directory:
            return "No such file or directory";
        case std::errc::no_such_process:
            return "No such process";
        case std::errc::not_a_directory:
            return "Not a directory";
        case std::errc::not_a_socket:
            return "Not a socket";
        case std::errc::not_a_stream:
            return "Not a STREAM";
        case std::errc::not_connected:
            return "Socket is not connected";
        case std::errc::not_enough_memory:
            return "Not enough space";
        case std::errc::not_supported:
            return "Not supported";
        case std::errc::operation_canceled:
            return "Operation canceled";
        case std::errc::operation_in_progress:
            return "Operation now in progress";
        case std::errc::operation_not_permitted:
            return "Operation not permitted";
        case std::errc::operation_not_supported:
            return "Operation not supported on socket";
        case std::errc::operation_would_block:
            return "Operation would block";
        case std::errc::owner_dead:
            return "Owner died";
        case std::errc::permission_denied:
            return "Permission denied";
        case std::errc::protocol_error:
            return "Protocol error";
        case std::errc::protocol_not_supported:
            return "Protocol not supported";
        case std::errc::read_only_file_system:
            return "Read-only file system";
        case std::errc::resource_deadlock_would_occur:
            return "Resource deadlock would occur";
        case std::errc::resource_unavailable_try_again:
            return "Resource unavailable, try again";
        case std::errc::result_out_of_range:
            return "Result out of range";
        case std::errc::state_not_recoverable:
            return "State not recoverable";
        case std::errc::stream_timeout:
            return "Stream timed out";
        case std::errc::text_file_busy:
            return "Text file busy";
        case std::errc::timed_out:
            return "Connection timed out";
        case std::errc::too_many_files_open_in_system:
            return "Too many files open in system";
        case std::errc::too_many_files_open:
            return "Too many open files";
        case std::errc::too_many_links:
            return "Too many links";
        case std::errc::too_many_symbolic_link_levels:
            return "Too many levels of symbolic links";
        case std::errc::value_too_large:
            return "Value too large to be stored in data type";
        case std::errc::wrong_protocol_type:
            return "Wrong protocol type for socket";
        default: return "Unknown error";
    }
}

StringView Win32ErrorCategory::GetName() const noexcept
{
    return "Win32";
}

String Win32ErrorCategory::MakeErrorMessage(
    const int _err) const
{
    ARRAY<wchar_t, 512> buf;
    const DWORD         written = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        static_cast<DWORD>(_err),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf.data(),
        static_cast<DWORD>(buf.size()),
        nullptr);

    if (written == 0)
    {
        return Format("Unknown Win32 error code: {}", _err);
    }

    return ToUtf8({ buf.data(), static_cast<size_t>(written) });
}

StringView HResultErrorCategory::GetName() const noexcept
{
    return "HRESULT";
}

String HResultErrorCategory::MakeErrorMessage(
    const int _err) const
{
    const HRESULT hr = static_cast<HRESULT>(_err);

    ARRAY<wchar_t, 512> buf;
    DWORD               written = 0;

    if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
    {
        written = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            static_cast<DWORD>(HRESULT_CODE(hr)),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf.data(),
            static_cast<DWORD>(buf.size()),
            nullptr);
    }

    if (written == 0)
    {
        written = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            static_cast<DWORD>(hr),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf.data(),
            static_cast<DWORD>(buf.size()),
            nullptr);
    }

    if (written == 0)
    {
        return Format("Unknown HRESULT error code: 0x{:08X}", static_cast<unsigned long>(hr));
    }

    return ToUtf8({ buf.data(), static_cast<size_t>(written) });
}

Error MakeSystemError(
    const int          _err,
    const eSystemError _type)
{
    switch (_type)
    {
        case eSystemError::Errno:
            return Error { _err, GetErrorCategory<ErrnoErrorCategory>() };
        case eSystemError::Win32:
            return Error { _err, GetErrorCategory<Win32ErrorCategory>() };
        case eSystemError::HRESULT:
            return Error { _err, GetErrorCategory<HResultErrorCategory>() };
        default:
            JUG_ASSERT(false, "Unrecognized system error type.\n");
            JUG_UNREACHABLE_RETURN(Error);
    }
}

Error MakeSystemError(
    const std::errc _err)
{
    return Error { static_cast<int>(_err), GetErrorCategory<ErrnoErrorCategory>() };
}

}   // namespace jug