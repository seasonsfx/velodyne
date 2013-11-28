// %pacpus:license{
// This file is part of the PACPUS framework distributed under the
// CECILL-C License, Version 1.0.
// %pacpus:license}
/// @file
/// @author  Gerald Dherbomez <firstname.surname@utc.fr>
/// @date    January, 2007
/// @version $Id: Win32ShMem.h 114 2013-06-25 08:55:43Z kurdejma $
/// @copyright Copyright (c) UTC/CNRS Heudiasyc 2006 - 2013. All rights reserved.
/// @brief Shared memory implementation for Windows.
///
/// Shared memory implementation for Windows.
/// @todo Derive from a common base class. Same for @link PosixShMem @endlink.

#ifndef DEF_PACPUS_WIN32SHMEM_H
#define DEF_PACPUS_WIN32SHMEM_H

#include "Pacpus\kernel\PacpusToolsConfig.h"

#include <Pacpus/PacpusTools/ShMemBase.h>

/// Windows handle (opaque pointer) type definition
/// 
/// Forward declaration of typedefs is impossible.
typedef void * HANDLE;

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4275)
#endif // _MSC_VER

namespace pacpus {

/// Shared memory object for Windows.
class PACPUSTOOLS_API Win32ShMem
        : public ShMemBase
{
public:
    /// Ctor
    Win32ShMem(const char * name, int size);
    /// Dtor
    ~Win32ShMem();

    /// @todo Documentation
    virtual bool wait(unsigned long timeout = 0);
    /// @todo Documentation
    virtual void * read();
    /// @todo Documentation
    virtual void read(void * mem, int size);
    /// @todo Documentation
    virtual void write(void * data, int size, unsigned long offset = 0);
    /// @todo Documentation
    virtual void lockMemory();
    /// @todo Documentation
    virtual void unlockMemory();
    /// @todo Documentation
    virtual void * getEventIdentifier();

protected:  

private:
    HANDLE semaphore_;
    HANDLE shMemHandle_;
    HANDLE event_;
};

} // namespace pacpus

#ifdef _MSC_VER
#   pragma warning(pop)
#endif // _MSC_VER

#endif // DEF_PACPUS_WIN32SHMEM_H