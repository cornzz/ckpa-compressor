# Technische Dokumentation vom FXPlugin Dynamics

## Gliederung

1. Systemvorraussetzungen
2. Verwendung
3. Bedienung
4. Systemarchitektur
	* Projektstruktur
		* Verzeichnisstruktur
	* Komponenten und Funktionen
		* FF_Meters
		* PluginProcessor
		* PluginEditor
		* Visualizer
		
## Systemvorraussetzungen

### Plattformabhängige Debug-Anforderungen

* macOS	
	* 10.9 oder höher
	* Xcode 7.3 oder höher
* Windows
	* 8.1 oder höher
	* Visual Studio 2015 oder höher
	* 64 Bit	
* Linux
	* Ubuntu 16.04.3 LTS
	* Makefile

### Allgemeine Debug-Anforderungen

* JUCE 5

## Verwendung

1. Visual Studio installieren
2. JUCE installieren
3. CKPA_Compressor.jucer im Projucer öffnen

## Bedienung

Das Plugin besteht aus 3 Levels, die jeweils in einem Tab angezeigt werden.
Um in ein bestimmtes Level zu wechseln, klicke den jeweiligen Tab mit der entsprechenden Level Beschriftung an.
Für eine ausführliche Beschreibung der Levels in der wissenschaftlichen Dokumentation nachschauen.

### Level 1

In diesem Level können die Parameter Threshold, Ratio, Attack, Release und Makeup Gain durch Bewegen des jeweiligen Schiebereglers beeinflusst werden. 

### Level 2

In diesem Level können nur Threshold, Ratio und Makeu Gain beeinflusst werden.
Das ist durch bewegen des jeweiligen Labels möglich (intern).

### Level 3

In diesem Level werden Threshold, Ratio und Makeup Gain beinflusst durch
Bewegen des Schiebereglers auf dem Rand des Kreises.

## Systemarchitektur

### Projektstruktur

#### Verzeichnisstruktur

* FXPlugin-Dynamics-SoSe20
	* Source
		* ff_meters (Ordner)                     -> fremde Bibliothek, enthält Darstellungsmöglichkeit für u.a. Makeup Gain
		* Editor Dateien                         -> Darstellung der einzelnen Levels
		* PluginProcessor und -Parameter Dateien -> Berechung und Darstellung der Parameter
		* Visualizer Dateien                     -> Visualierung des Signals

### Komponenten und Funktionen

#### Plugin Editor

Die Komponente übernimmt die Darstellung der einzelnen Levels. 
Dafür besitzt sie ein TabbedComponent Objekt, welches mit Instanzen der Klassen LevelXEditor (X = 1, 2, 3) initialisiert wird.
Die Levels sind von einander unabhängige Objekte, initialisieren ihre eigenen Komponenten und haben eine eigene Referenz zum PluginProcessor.

#### Plugin Processor

Diese Komponente berechnet alle Parameter: "Threshold, Ratio, Attack, Release und Makeup Gain".
Außerdem existiert noch ein "Bypass" - Parameter, wodurch keine Veränderungen am Signal ausgeführt werden.
Der Parameter "Compression" wurde für das Level 3 eingeführt, um Threshold, Ratio und Makeup Gain in
einem Wert zuvereinen.

#### Plugin Parameter Manager

Der Plugin Paramter Manager verwaltet die Parameter Daten durch
Speicherung und Verknüpfung mit den dazugehörigen User Interface Elementen.

#### Visualizer

Die Komponente übernimmt die Darstellung des Signals in Waveform, welche in Level 2 verwendet wird.
