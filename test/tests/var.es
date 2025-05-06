# this test assumes %#L uses %S for each string.
test '%var quoting/escaping' {
	assert {local (x = '''') {
		~ <={%var x} x[=\ ]*''''''
	}} 'empty string'
	assert {local (x = \1) {
		~ <={%var x} x[=\ ]*'\1'
	}} 'unprintable char is escaped'
	assert {local (x = unquoted) {
		~ <={%var x} x[=\ ]*'unquoted'
	}} 'unquoted string'
	assert {local (x = 'quoted 'string) {
		~ <={%var x} x[=\ ]*'''quoted string'''
	}} 'quoted string'
	assert {local (x = '@') {
		~ <={%var x} x[=\ ]*'''@'''
	}} '@ is quoted'
	assert {local (x = %) {
		~ <={%var x} x[=\ ]*'%'
	}} '% is unquoted'
	assert {local (x = %\1^unquoted) {
		~ <={%var x} x[=\ ]*'%^\1^unquoted'
	}} '%+unprintable+unquoted'
	assert {local (x = unquoted\1^'quoted string') {
		~ <={%var x} x[=\ ]*'unquoted^\1^''quoted string'''
	}} 'unquoted+unprintable+quoted'
	assert {local (x = %forcibly'@'quoted) {
		~ <={%var x} x[=\ ]*'''%forcibly@quoted'''
	}} '@ forces quoting of adjacent printable strings'
}
