//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at CSEE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------


---------------------------------------------
// Readout of QFW beam diagnostic board via PEXOR family optical receiver.
// V 0.9 11-Dec-2013
// Joern Adamczewski-Musch, CSEE, GSI Darmstadt
// j.adamczewski@gsi.de
---------------------------------------------



Note: English description currently not available, sorry. 
We will translate the German explanations as soon as possible JAM

##############################################################################
---------------------------------------------------------------------------
Usage of this software for QFW test:

1) Ist installiert auf sdpc095.gsi.de als user mbsdaq im Verzeichnis
~/go4_qfw/hd/pexor. Das zugeh�rige Repository (zur Installation auf beliebigem Rechner mit Go4 framework) ist erreichbar unter
https://subversion.gsi.de/go4/app/qfw/pexor

***********************************************************************
2) Starten:
Nach ". go4login"
mit "cd /LynxOS/mbsusr/mbsdaq/go4_qfw/hd/pexor" ins Arbeitsverzeichnis wechseln.

Dort gibt ein hotstart script das als default das lmd file
/data/HIT_run12_0001.lmd �ffnet:
"go4 hit" startet dieses. (Die Analyse ist hier am Anfang gestoppt, d.h. nochmal auf gr�nen Pfeil dr�cken!)

*********************************************************************** 

3) Setup:

Die Verkabelung der qfw boards per sfp zum pexor wird in der Klasse
TQFWRawParam �ber die Arrays fBoardID[sfp][dev] eingestellt.
Dabei ist
    sfp: sfp Strang am Pexor (0,..,3)
    dev: device nummer an diesem Strang (0,...,255?)
    fBoardID: eine (hier willk�rliche) eindeutige Ger�teID des QFW boards. F�r den HIT Aufbau sind hier die ids 10,11, 12 gew�hlt. Sp�ter sollten diese Nummern irgendwie auf der Board hardware zu finden sein...
Funktion TQFWRawParam::InitBoardMapping() in TQFWRawParam.cxx initialisiert dies mit dem Aufbau am HIT

Die Verkabelung der qfw boards zu den grids/faraday cups etc ist in der Klasse TQFWProfileParam eingestellt mit Parameter Arrays �ber einen laufenden index aller Profilgitter (grid) und Gitterdr�hten (wire):

 fNumGrids - Anzahl aller Gitter im Aufbau

 fGridDeviceID[grid] - Unique id des Gitters am laufenden index grid. Diese ist zun�chst willk�rlich, sollte sp�ter aber auf der hardware wiederzufinden sein.


 fGridBoardID_X[grid][wire] -
    Unique id des qfw boards (s.o.) an Gitter grid, Draht wire in X Richtung

 fGridBoardID_Y[grid][wire] -
Unique id des qfw boards an Gitter grid, Draht wire in Y Richtung

 fGridChannel_X[grid][wire] - Nummer des qfw board Kanals (0...32) f�r Gitter grid, Draht wire in X

 fGridChannel_Y[grid][wire] -  Nummer des qfw board Kanals (0...32) f�r Gitter grid, Draht wire in Y


Neben den Strahlprofilgittern k�nnen "Cup" Objekte konfiguriert werden, d.h. irgendwie segmentierte Ladungsmessungsplatten. Dies geschieht mit Parametern �ber indices der Segmentierten Einheiten (cup) und der Segmentnummer (seg)

fNumCups - Anzahl aller "Segmentierten Cups"

fCupDeviceID[cup] -
    Unique id des Cup Devices f�r index cup
fCupBoardID[cup][seg]-
    Unique id des qfw boards an Cup cup, Segment seg
 fCupChannel[cup][seg] -
      Nummer des qfw board Kanals (0...32) an Cup cup, Segment seg

Negative Werte f�r diese Arrays deaktiviert den Entsprechenden Kanal!

Die Konfiguration der Time Slices passiert automatisch anhand der eingelesenen Daten!

F�r die HIT Strahlzeit gibt es hier ein Gitter mit willk�rlicher ID 42
(verbunden mit boards 10 und 11) und ein "Cup" mit id 66 und 2 Segmenten, das sind die schr�g segmentierten Kondensatorplatten (?)


*************************************************************************
4) Histogramm Displays:

Die Go4 Analyse hat nun 2 Stufen (Analysis Steps):
Die erste Stufe packt die Rohdaten von der DAQ aus und zeigt die QFW Board Kan�le. Deren Histogramme befinden sich unter Histograms/Board%d
(also in unserem Fall Board10, Board11, Board12 f�r die von mir willk�rlich vergebenen "unique ids").

Die zweite Stufe mappt die qfw Kan�le entsprechend der oben beschriebenen Zuweisungen auf die messenden Ger�te, also Gitter oder segmentierte Platten/Cups. Deren Histogramme sind unter
Histograms/Beam/Grid42 bzw Histograms/Beam/Cup66 zu finden.

F�r jedes board/grid/cup gibt es Unterordner Loop0,Loop1,Loop2, die spezifische Histogramme f�r den jeweiligen Zeitloop enthalten.


Besondere Histogramme f�r grids:
Meanpos_G42: Statistik des mean values �ber alle x/y Profile. Das sollte eine Art Strahlpositon in x/y wiedergeben

RMS_X_G42: Statistik des rms values �ber alle x/y Profile in x. Das sollte eine Art rms Strahldurchmesser in x ergeben

RMS_Y_G42: Statistik des rms values �ber alle x/y Profile in x. Das sollte eine Art rms Strahldurchmesser in x ergeben


Besondere Histogramme f�r segmentierte cups:
Cup66/Loop1/SegmentRatio_C66_L1_Si : Verh�ltnis der Ladung auf Segmenti zur Ladung auf allen Segmenten (f�r dieses Event) dito f�r andere Timeloops

Cup66/Loop1/SegmentRatioSum_C66_L1_Si : Verh�ltnis der Ladung auf Segmenti zur Ladung auf allen Segmenten (akkumuliert f�r alle Events). dito f�r andere Timeloops

Dies kann zur Positionsbestimmung des Strahls relativ zu den Segmenten benutzt werden.


Pictures:
Mehrere Histogramme f�r ein Device sind in "Go4 Pictures" zusammengefasst:
z.B.
Pictures/Board10/QFW_Rawscalers_Brd10 - alle scaler f�r board 10 Kan�le

Pictures/Beam/Grid42/Beam Display Grid42
- Projektionen des "Strahlprofils" in x/y

Pictures/Beam/Grid42/Beam RMS Grid42
- RMS "Strahldurchmesser" (s.o.) in x und y


**************************************************************************
5) Besondere Analysemodi:

--------------------------------------------------------------------------
A) Messung des Offsets und die Korrektur
Interaktiv einstellbar im Parameter Parameters/QFWProfileParam (Doppelklick auf Go4 browser �ffnet Parameter Editor).

Zum Starten der Offset Messung "fMeasureBackground" im Parameter Editor auf 1 (true) setzen, dann Parameter mit "Return" und Pfeil nach links Knopf" aktivieren und lmd file mit Offset einlesen. Sobald Offset Messung beendet ist "fMeasureBackground" wieder auf 0 setzen  und lmd file mit beamdaten einlesen. Dann sollte der offset in der Darstellung der _grids_ und der _cups_ abgezogen werden (! die Board histogramme werden nie korrigiert!)

Untergrundkorrektur kann zus�tzlich mit
"fCorrectBackground" ein und ausgeschaltet werden.

Zur Kontrolle der gerade zur Korrektur benutzten Offsets dienen Histogramme
"Beam/Grid42/Loop 1/Profile_X_Time_Offset_G42_L1" (analog f�r andere timeloops und grid ids)
bzw.
Beam/Cup66/Loop 1/Scaler_Time_Offset_C42_L1 (dito)

---------------------------------------------------------------------------
B) Slow motion:
Interaktiv einstellbar im Parameter Parameters/QFWProfileParam (Doppelklick auf Go4 browser �ffnet Parameter Editor).
"fSlowMotionStart" auf 0: Normaler Betrieb
Setzen von
"fSlowMotionStart" auf eine Eventnummer aktiviert Zeitlupenmodus f�r alle Events mit gr��erer Laufender Nummer, d.h. nach jeweils einem Event wird die Analyse angehalten und man kann nach Refresh des Go4 Displays (F5) sich den aktuellen Trace ansehen. Dr�cken des Startknopfes (gr�ner Pfeil oder CTRL-S) holt dann das n�chste Event, usw.
Achtung: Laufende Nummer ist die vom QFW in den Daten eingeschriebene Sequenznummer, nicht die Nummer der Go4 Events!

------------------------------------------------------------------------
C) �berspringen von "schlechten Events" bei freilaufender DAQ

Ziel: Anhand der Ladung auf ausgew�hlten Kan�len soll entschieden werden, ob es sich um ein Strahlevent (Strahl voll im Setup), ein Untergrundevent (kein Strahl), oder ein "schlechtes Event" (Strahl halb drin) handelt

Einstellbar im Parameter Parameters/QFWRawParam:

fSelectTriggerEvents; // switch triggering of good and background events for free running daq

fTriggerHighThreshold; // selects good event if integral of trigger channel counts is above

fTriggerLowThreshold; // selects background events if integral of trigger channel counts is below

fTriggerBoardID; // id of board that contains trigger channels

fTriggerFirstChannel; // lower boundary of trigger channel region

fTriggerLastChannel; // upper boundary of trigger channel region

Welche QFW Kan�le f�r die Triggerentscheidung aufsummiert werden, wird mit fTriggerBoardID (unique id des QFW boards, siehe oben) und einem
Kanalbereich
fTriggerFirstChannel <= qfwchannel <= fTriggerLastChannel
eingestellt.
F�r das hit setup ist hier der default eingestellt
in TQFWRawParam::InitBoardMapping() -
fTriggerBoardID=12, fTriggerFirstChannel=0, fTriggerLastChannel=1


Durchgelassen werden hier nur die beam events (Summe der Ladung ist oberhalb von fTriggerHighThreshold) und die background events
(Summe der Ladung ist unterhalb von fTriggerLowThreshold). Dazwischenliegende halbe Events werden �bersprungen. Man erh�lt f�r jedes �bersprungene Event eine Meldung im Analyseterminal:
GO4-*> Skip event of seqnr 9327 with triggersum 4466225!
(mit QFW event sequenznummer und Ladungssumme der Triggerkan�le)

************************************************************************ 


JAM 11-Dec-2013