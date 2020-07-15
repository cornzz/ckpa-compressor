#Technische Dokumentation vom FXPlugin Dynamics

##Gliederung

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
		
##Systemvorraussetzungen

### Plattformabhängige Anforderungen

* macOS	
	* 10.9 or higher
	* Xcode 7.3 or higher	
* Windows
	* 8.1 or higher
	* Visual Studio 2015 or higher
	* 64 Bit	
* Linux
	* Ubuntu 16.04.3 LTS
	* Makefile

### Allgemeine Anforderungen

* JUCE 6

##Verwendung

1. Visual Studio installieren
2. JUCE installieren
3. 

##Bedienung

Das Plugin besteht aus 3 Levels, die jeweils in einem Tab angezeigt werden.
Um in ein bestimmtes Level zu wechseln, klicke den jeweiligen Tab mit der
entsprechenden Level Beschriftung an.
Für eine ausführliche Beschreibung der Levels in der wissenschaftlichen Dokumentation nachschauen.

### Level 1

In diesem Level können die Parameter Threshold, Ratio, Attack, Release und Makeup Gain
beeinflusst werden durch Bewegen des jeweiligen Sliderknopfes.

### Level 2

In diesem Level können nur Threshold, Ratio und Makeu Gain beeinflusst werden.
Das wird erreicht durch Bewegen jeweiligen Labels (intern ist ein Slider).

### Level 3

In diesem Level werden Threshold, Ratio und Makeup Gain beinflusst durch
Bewegen des Sliderknopfes auf dem Rand des Kreises.

##Systemarchitektur

###Projektstruktur

####Verzeichnisstruktur

* FXPlugin-Dynamics-SoSe20
	* Source
		* ff_meters (Ordner)                     -> fremde Bibliothek, enthält Darstellungsmöglichkeit für u.a. Makeup Gain
		* Editor Dateien                         -> Darstellung der einzelnen Levels
		* PluginProcessor und -Parameter Dateien -> Berechung und Darstellung der Parameter
		* Visualizer Dateien                     -> Visualierung des Signals

###Komponenten und Funktionen

####Plugin Editor

Die Komponente übernimmt die Darstellung der einzelnen Levels. 
Jedes einzelne Level ist ein Editor, welche in einem Tab angezeigt wird.
Und jede dieser Editoren initialisiert seine eigenen Komponenten.

####Plugin Processor

Diese Komponente berechnet alle Parameter: "Threshold, Ratio, Attack, Release und Makeup Gain".
Außerdem existiert noch ein "Bypass" - Parameter, wodurch keine Veränderungen am Signal ausgeführt werden.
Der Parameter "Compression" wurde für das Level 3 eingeführt, um Threshold, Ratio und Makeup Gain in
einem Wert zuvereinen.

####Plugin Parameter Manager

Der Plugin Paramter Manaager verwaltet die Parameter Daten durch
Speicherung und Verknüpfung mit den dazugehörigen User Interface Elementen.

####Visualizer

Die Komponente übernimmt die Darstellung des Signals in Waveform, welche in Level 2 verwendet wird.




		
	

