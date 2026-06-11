/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  NamedMutex.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#include "NamedMutex.h"

//the "locker" names exist for call-site context and debugging; ownership tracking is left
//entirely to the underlying recursive mutex (the old name-string comparison was a data race)
void NamedMutex::Lock(std::string /*locker*/)
{
   mMutex.lock();
}

bool NamedMutex::TryLock(std::string /*locker*/)
{
   return mMutex.try_lock();
}

void NamedMutex::Unlock()
{
   mMutex.unlock();
}

ScopedMutex::ScopedMutex(NamedMutex* mutex, std::string locker)
: mMutex(mutex)
{
   mMutex->Lock(std::move(locker));
}

ScopedMutex::~ScopedMutex()
{
   mMutex->Unlock();
}

ScopedTryMutex::ScopedTryMutex(NamedMutex* mutex, std::string locker)
: mMutex(mutex)
, mAcquired(mutex->TryLock(std::move(locker)))
{
}

ScopedTryMutex::~ScopedTryMutex()
{
   if (mAcquired)
      mMutex->Unlock();
}