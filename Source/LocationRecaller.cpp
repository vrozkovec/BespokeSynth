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
/*
  ==============================================================================

    LocationRecaller.cpp
    Created: 13 Apr 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "LocationRecaller.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

LocationRecaller::LocationRecaller()
: IDrawableModule(150, 40)
{
}

LocationRecaller::~LocationRecaller()
{
}

void LocationRecaller::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mLocationSelector, "location", &mLocationIndex, 50);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mSaveModeCheckbox, "save", &mSaveMode);
   ENDUIBLOCK(mWidth, mHeight);

   mLocationSelector->AddLabel("1", 0);
   mLocationSelector->AddLabel("2", 1);
   mLocationSelector->AddLabel("3", 2);
   mLocationSelector->AddLabel("4", 3);
   mLocationSelector->AddLabel("5", 4);
   mLocationSelector->AddLabel("6", 5);
   mLocationSelector->AddLabel("7", 6);
   mLocationSelector->AddLabel("8", 7);
   mLocationSelector->AddLabel("9", 8);
}

void LocationRecaller::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mLocationSelector->Draw();
   mSaveModeCheckbox->Draw();

   if (gTime - mLastTriggerTime > 0 && gTime - mLastTriggerTime < 200)
   {
      ofPushStyle();
      ofSetColor(0, 255, 0, ofMap(gTime - mLastTriggerTime, 0, 200, 255, 0));
      ofFill();
      ofRect(mWidth - 13, 2, 10, 7);
      ofPopStyle();
   }
}

void LocationRecaller::GetModuleDimensions(float& width, float& height)
{
   width = 150;
   height = 40;
}

void LocationRecaller::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);

   if (!mEnabled)
      return;

   TriggerLocation(mLocationIndex);
}

void LocationRecaller::PlayNote(NoteMessage note)
{
   if (!mEnabled || note.velocity <= 0)
      return;

   int index = note.pitch % 9;
   TriggerLocation(index);
}

void LocationRecaller::TriggerLocation(int index)
{
   char key = '1' + index;

   if (mSaveMode)
      TheSynth->GetLocationZoomer()->WriteCurrentLocation(key);
   else
      TheSynth->GetLocationZoomer()->MoveToLocation(key);

   mLastTriggerTime = gTime;
}

void LocationRecaller::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void LocationRecaller::SetUpFromSaveData()
{
}
