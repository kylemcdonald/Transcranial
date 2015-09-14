# Transcranial

Interactive dance performance developed during Resonate 2014 with Daito Manabe and Klaus Obermaier.

## Network connection at Resonate 2015

Make sure to turn firewall off in security & privacy, or ping will not work.

- Klaus: 10.0.0.1 (USB ethernet)
- Daito: 10.0.0.2 (Thunderbolt ethernet)
- Kyle: 10.0.0.3 (Thunderbolt ethernet)

## General Controls

* `f` is fullscreen
* `g` is fullscreen on second screen
* `tab` shows debug

## Before the performance: Faces

Before the performance we photograph the faces of the audience. They should be shot against a solid color background with ambient light from in front or above.

1. Delete everything in `faces/` and `meshes/`.
2. Drop faces (1920x1080) into `raw/`.
3. Run `resize-faces.command`, this resizes and moves them into `faces/`.
4. Run `ProcessCrowdFaces`, this analyzes every photo in `faces/` for up to 2 seconds each and removes and faces that cannot be detected. This generates face meshes in `meshes/`.

## Scene 1: Piripiri

No interaction, just control. Daito leads this scene with the dancer seating at the front of the stage wearing the piripiri electrodes.

- Audio: from Daito, in Max
- Video: from Kyle, in Max `FaceCopyWithPiripiri/maxpatches/main_copy_piripiri.maxpat`

## Scene 2: Elephants

No interaction. Duet in swimsuits on two chairs.

- Audio: from Klaus, in Max.
- No video.

## Scene 3: Body

Stage right near screen, three parts of improvisation.

- Audio: from Klaus.
- Video: from Kyle on main screen, using secondary projector for lighting the dancer. Video from the Canon 5d on the floor, long extension cable running to the right USB port.

### Controls

* backspace: stabilityFade.stop()
* 1: loadScene1
* 2: loadScene2
* 3: loadScene3

## Scene 4: Face

Stage right near screen. Wireless lav mic on the interviewee, handheld mic for the interviewer.

- Audio: from Klaus.
- Video: from Kyle on main screen. Video from the Canon T2i with long lens, at eye level, at stage left front, near the table. Short extension cable running to the right USB port.

### Controls

* shift: start/stop offset
* .: load next face / start face substitution
* q: soft
* w: medium
* e: heavy
* c: low learning rate
* backspace: stop face substitution [not used]


## Scene 5: Stripes

No interaction. Solo in center of the stage.

- Audio: from Klaus.
- Video: from Daito, in Max, on main screen. From Klaus, in Max, on two side projectors.

## Todo

- Record Daito's video.
- Check that I can run Daito's video.
- Get lighting for Face Substitution working.