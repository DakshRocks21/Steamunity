# Steamunity 2024: Creating a Doorbell System for elderlies with Hearing Impairments
Welcome! This is a project made for the Steamunity: Design for Community Challenge hosted by SUTD, Singapore. \
Our group consists of Daksh, Jarrett, Jason, Xiu Ling, Tiffany, and Richard, and we have designed an innovate doorbell system to help elderlies who have hearing impairments.

### A Deeper Dive into the Problem
Out of the 15 problem statements offered by the competition organisers, our group chose to work with Chinatown Active Aging Center (AAC) to create a product to help seniors in the area. \

The AAC provides support for seniors in the area through community engagement and befriending activities, and one aspect of these services include home visits. However, a sizable portion of their beneficiaries encounter hearing issues due to their old age, and social workers from Chinatown AAC face difficulty getting them to answer the door, sometimes waiting outside for as long as 30 minutes. It frustrates them greatly and prevents elderlies to receive the support they deserve, and our group aims to improve communication between the two parties with our custom-made doorbell.

### We thus present... DUO
Duo is named as such for not just one, but two reasons:
1. Duo consists of two main components, the doorbell and a wristband. Whenever Duo's doorbell is pressed, it sends a signal to the wristband and activates it, sending visual, audio and vibration cues, compelling wearers to answer the door swiftly. Furthermore, the doorbell comes with a screen! It keeps visitors in the know on whether the homeowner has acknowledged, and likewise, the wristband comes with an acknowledge button. 
2. We designed this product with not just the elderly residents in mind, but also visitors; the acknowledgement feature lets visitors know whether the elderly is on their way to the door and reduces frustrations they might face.

Duo makes use of ESPs (ESP Wemos Lolin and Seeed Xiao C3) to communicate between its various modules using ESP-NOW protocol. It does not use Wifi and makes setting up the system much easier. We also decided to use a range extender in our project (another ESP) that allows the doorbell and wristband to communicate with each other over longer distances, or through more walls.

Ok, cool. The solution works. But what if the elderlies do not want to use it? /

Our team's aware that some elderlies just don't like wearing a wristband all the time when they're at home. That's all right! We designed the wristband to be loud and flashy enough to catch the elderly's attention even if its not strapped to their wrist. (So long as they place it near enough to them, like a bedside table, it _should_ be fine :D )

### Room for Improvement
This product is a rough prototype made for ***Round 1*** of this competition, meaning there's a second round! Our group intends to join this, where our product will be refined and scaled up to something that's actually implementable in the community. And I personally love that idea. \
Here's some cool things we aim to do:
- More customisability: Ringtone, Volume, Vibration Strength, Doorbell Screen Messages, etc. the list goes on
- More features: Making the wristband look and work more like a smartwatch, so the elderlies are more likely to use it on a daily basis
- Backups: We'll refine the backup attention grabber feature thingy whatever it is (like if they are rly stubborn and STILL don't want to use the wristband)
- And we'll conduct more research and make our design sleeker and cooler overall.
You get the gist.

### Misc. References
Decided to keep these here cause why not, lol
More about AACs in Singapore: https://supportgowhere.life.gov.sg/services/SVC-AACAAACHASACS/active-ageing-centres-aac \
Buzzer Music Tutorial: https://esp32io.com/tutorials/esp32-piezo-buzzer \
LCD Screen Tutorial: https://www.youtube.com/watch?v=COssWn4Pcm4 \
Subway Surfers Ringtone: https://github.com/hibit-dev/buzzer/blob/master/src/movies/subway_surfers/subway_surfers.ino
