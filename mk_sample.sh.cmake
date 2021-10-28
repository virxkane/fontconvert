#!/bin/sh

# NotoSans Regular, 7pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Regular.ttf > NotoSans-Regular-7pt-ascii.h

# NotoSans Regular, 7pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Regular.ttf > NotoSans-Regular-7pt-ascii+degree+rus.h

# NotoSans Regular, 7pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Regular.ttf > NotoSans-Regular-7pt-digits+punct+degree+rus.h

# NotoSans Bold, 7pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Bold.ttf > NotoSans-Bold-7pt-ascii.h

# NotoSans Bold, 7pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Bold.ttf > NotoSans-Bold-7pt-ascii+degree+rus.h

# NotoSans Bold, 7pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Bold.ttf > NotoSans-Bold-7pt-digits+punct+degree+rus.h

# NotoSans Italic, 7pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Italic.ttf > NotoSans-Italic-7pt-ascii.h

# NotoSans Italic, 7pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Italic.ttf > NotoSans-Italic-7pt-ascii+degree+rus.h

# NotoSans Italic, 7pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Italic.ttf > NotoSans-Italic-7pt-digits+punct+degree+rus.h

# NotoSans BoldItalic, 7pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-BoldItalic.ttf > NotoSans-BoldItalic-7pt-ascii.h

# NotoSans BoldItalic, 7pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-BoldItalic.ttf > NotoSans-BoldItalic-7pt-ascii+degree+rus.h

# NotoSans BoldItalic, 7pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=7 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-BoldItalic.ttf > NotoSans-BoldItalic-7pt-digits+punct+degree+rus.h


# NotoSans Regular, 14pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Regular.ttf > NotoSans-Regular-14pt-ascii.h

# NotoSans Regular, 14pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Regular.ttf > NotoSans-Regular-14pt-ascii+degree+rus.h

# NotoSans Regular, 14pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Regular.ttf > NotoSans-Regular-14pt-digits+punct+degree+rus.h

# NotoSans Bold, 14pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Bold.ttf > NotoSans-Bold-14pt-ascii.h

# NotoSans Bold, 14pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Bold.ttf > NotoSans-Bold-14pt-ascii+degree+rus.h

# NotoSans Bold, 14pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Bold.ttf > NotoSans-Bold-14pt-digits+punct+degree+rus.h

# NotoSans Italic, 14pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Italic.ttf > NotoSans-Italic-14pt-ascii.h

# NotoSans Italic, 14pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Italic.ttf > NotoSans-Italic-14pt-ascii+degree+rus.h

# NotoSans Italic, 14pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-Italic.ttf > NotoSans-Italic-14pt-digits+punct+degree+rus.h

# NotoSans BoldItalic, 14pt, DPI 116, ASCII only
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --ascii --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-BoldItalic.ttf > NotoSans-BoldItalic-14pt-ascii.h

# NotoSans BoldItalic, 14pt, DPI 116, ASCII, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-BoldItalic.ttf > NotoSans-BoldItalic-14pt-ascii+degree+rus.h

# NotoSans BoldItalic, 14pt, DPI 116, digits, punctuation marks, degree sign, Russian Cyrillic
${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} \
	--size=14 --chars=0x20-0x40,0x7E,0x410-0x44F,0x401,0x451,0xB0 --dpi=116 --hinting=auto \
	/usr/share/fonts/noto/NotoSans-BoldItalic.ttf > NotoSans-BoldItalic-14pt-digits+punct+degree+rus.h
