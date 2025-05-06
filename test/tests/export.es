test 'exported names' {
	assert {for (b = (
		A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
		a b c d e f g h i j k l m n o p q r s t u v w x y z
		_ _0 _1 _2 _3 _4 _5 _6 _7 _8 _9
	)) {
		if {!~ ``^ \n {
			local ($b = unique-string) {env | grep unique-string}
		} $b[=\ ]*'unique-string'} {
			break <=false
		}
	}} 'POSIX-compatible variable names unescaped'
	assert {for (b = (
		   # \x2a is '*', which is noexport -- \x2a_ is not noexport.
		    01  02  03  04  05  06  07  08  09  0a  0b  0c  0d  0e  0f
		10  11  12  13  14  15  16  17  18  19  1a  1b  1c  1d  1e  1f
		20  21  22  23  24  25  26  27  28  29  2a_ 2b  2c  2d  2e  2f
		                                        3a  3b  3c  3d  3e  3f
		40
		                                            5b  5c  5d  5e
		60
		                                            7b  7c  7d  7e  7f
	)) {
		if {!~ ``^ \n {
			$es -c 'local (\x'$b' = unique-string) {env | grep unique-string}'
		} __$b[=\ ]*'unique-string'} {
			break <=false
		}
	}} 'non-POSIX ASCII variable names escaped'
	assert {local (\xa0 = nbsp) {
		~ `{env | grep nbsp} __a0[=\ ]*nbsp
	}} 'ISO/IEC 8859 non-ASCII bytes escaped'
	assert {local (\xce\xbb = lambda) {
		~ `{env | grep lambda} __ce__bb[=\ ]*lambda
	}} 'UTF-8 non-ASCII bytes escaped'
	assert {local (\x81\x30\x89\x38 = esszett) {
		~ `{env | grep esszett} __810__898[=\ ]*esszett
	}} 'GB18030 non-ASCII bytes escaped'
	assert {local (_a0 = uscore) {
		~ `{env | grep uscore} _a0[=\ ]*uscore
	}} 'leading underscore'
	assert {local (___a = uscore) {
		~ `{env | grep uscore} __5f__5f_a[=\ ]*uscore
	}} 'consecutive underscores'
}
