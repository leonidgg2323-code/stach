#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/core_test.cpp -o .build/core_test
.build/core_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/board_test.cpp -o .build/board_test
.build/board_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/face_test.cpp -o .build/face_test
.build/face_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/character_test.cpp -o .build/character_test
.build/character_test
c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/eyes_upgrade_test.cpp -o .build/eyes_upgrade_test
.build/eyes_upgrade_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/reference_style_test.cpp -o .build/reference_style_test
.build/reference_style_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/smart_behavior_test.cpp -o .build/smart_behavior_test
.build/smart_behavior_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/touch_gestures_test.cpp -o .build/touch_gestures_test
.build/touch_gestures_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/pairing_test.cpp -o .build/pairing_test
.build/pairing_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/control_test.cpp -o .build/control_test
.build/control_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/mouse_test.cpp -o .build/mouse_test
.build/mouse_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/power_test.cpp -o .build/power_test
.build/power_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/modes_test.cpp -o .build/modes_test
.build/modes_test

c++ -std=c++17 -Wall -Wextra -Werror -Iinclude test/security_test.cpp -o .build/security_test
.build/security_test
