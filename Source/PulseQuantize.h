/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
/*
  ==============================================================================

    PulseQuantize.h
    Created: 3 Jun 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"

class PulseQuantize : public IDrawableModule, public IPulseSource, public IPulseReceiver, public ITimeListener, public IDropdownListener
{
public:
   PulseQuantize();
   virtual ~PulseQuantize();
   static IDrawableModule* Create() { return new PulseQuantize(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;

   void Arm(float velocity, int flags);

   NoteInterval mInterval{ NoteInterval::kInterval_4n };
   DropdownList* mIntervalSelector{ nullptr };

   bool mArmed{ false };
   Checkbox* mArmedCheckbox{ nullptr };

   float mPendingVelocity{ 1 };
   int mPendingFlags{ 0 };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
