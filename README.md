# QuotidianMoon
An Arduino project showing the current Moon Phase on a small eInk display using an *LDR* rather than an RTC or an Internet connection.

<img width="768" height="1024" alt="image" src="https://github.com/user-attachments/assets/22032830-c652-4a4e-87f4-20e549ee7f6f" />

The phase above is shown as seen in the Southern Hemisphere. That choice, along with several other things are configurable.
The phase is approximate, based on the date and time, a configured reference New Moon and a fixed nominal lunar period.
The code keeps track of the approximate time and date using the Atmega32U4's built-in oscillator.  This will cause *unbounded* drift.

To keep the *drift constrained*, the code monitors the LDR. Every minute the LDR reading (i.e. ambient light) is classified as Dark or Light (Night or Day). These values are averaged into 10-minute intervals also classified as Dark or Light.

When this monitoring code detects a Light interval that was preceded by sufficiently long period of Dark intervals it concludes that *dawn* has occurred. It assumes the Dark period was night and that *midnight* was the midpoint.  It adjust the approximate time accordingly. The date will have already advanced.  Note that there are sanity checks applied to the adjustment and shift factors.

The time will always be a little off, but for our purposes that doesn't matter -- the date is the important thing and it should never deviate.

<img width="768" height="1024" alt="image" src="https://github.com/user-attachments/assets/11c4ace8-8a8a-45d7-aa90-eb7f0d9a3a5b" />

There's a small 128x32 OLED display and two push-buttons for configuration etc (the eInk is too slow for interaction).  Two other faces are available: a calendar and just the current date.

<img width="768" height="1024" alt="image" src="https://github.com/user-attachments/assets/7a6a919d-5950-419f-a187-4c7477232696" />


**For many many more details see the bulk comment in **QuotidianMoon.ino**.**

---
Hackaday project: [QuotidianMoon](https://hackaday.io/project/206893-quotidianmoon)

More photos at Flickr: [QuotidianMoon](https://flic.kr/s/aHBqjD6XjF)

YouTube video: [QuotidianMoon](https://youtu.be/cn6hoSS19BQ)

