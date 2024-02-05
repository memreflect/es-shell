# tests/trip.es -- migration of the classic trip.es to the new test framework.

test 'es -c' {
	assert {!$es -c >[2] /dev/null} 'didn''t report a bad exit status'
	assert {~ `` \n {$es -c 'echo $0 $2 $#*' a b c d e f} <={%flatten ' ' $es b 6}}
}

test 'lexical analysis' {
	# TODO: test warning for  

	echo here_is_a_really_long_word.It_has_got_to_be_longer_than_1000_characters_for_the_lexical_analyzers_buffer_to_overflow_but_that_should_not_be_too_difficult_to_do.Let_me_start_writing_some_Lewis_Carroll.Twas_brillig_and_the_slithy_toves,Did_gyre_and_gimble_in_the_wabe.All_mimsy_were_the_borogoves,And_the_mome-raths_outgrabe.Beware_the_Jabberwock_my_son,The_jaws_that_bite,the_claws_that_catch.Beware_the_Jub-jub_bird,and_shun_The_frumious_Bandersnatch.He_took_his_vorpal_sword_in_hand,Long_time_the_manxome_foe_he_sought,So_rested_he_by_the_Tumtum_tree,And_stood_awhile_in_thought.And_as_in_uffish_thought_he_stood,The_Jabberwock,with_eyes_of_flame,Came_whiffling_through_the_tulgey_wood,And_burbled_as_it_came.One_two,one_two.And_through_and_through_The_vorpal_blade_went_snicker-snack.He_left_it_dead_and_with_its_head,He_went_galumphing_back.And_hast_thou_slain_the_Jabberwock?Come_to_my_arms,my_beamish_boy,Oh_frabjous_day.Callooh_callay.He_chortled_in_his_joy.Twas_brillig,and_the_slithy_toves,Did_gyre_and_gimble_in_the_wabe,All_mimsy_were_the_borogoves,And_the_mome-raths_outgrabe. > /tmp/$pid.lw

	echo 'here_is_a_really_long_word.It_has_got_to_be_longer_than_1000_characters_for_the_lexical_analyzers_buffer_to_overflow_but_that_should_not_be_too_difficult_to_do.Let_me_start_writing_some_Lewis_Carroll.Twas_brillig_and_the_slithy_toves,Did_gyre_and_gimble_in_the_wabe.All_mimsy_were_the_borogoves,And_the_mome-raths_outgrabe.Beware_the_Jabberwock_my_son,The_jaws_that_bite,the_claws_that_catch.Beware_the_Jub-jub_bird,and_shun_The_frumious_Bandersnatch.He_took_his_vorpal_sword_in_hand,Long_time_the_manxome_foe_he_sought,So_rested_he_by_the_Tumtum_tree,And_stood_awhile_in_thought.And_as_in_uffish_thought_he_stood,The_Jabberwock,with_eyes_of_flame,Came_whiffling_through_the_tulgey_wood,And_burbled_as_it_came.One_two,one_two.And_through_and_through_The_vorpal_blade_went_snicker-snack.He_left_it_dead_and_with_its_head,He_went_galumphing_back.And_hast_thou_slain_the_Jabberwock?Come_to_my_arms,my_beamish_boy,Oh_frabjous_day.Callooh_callay.He_chortled_in_his_joy.Twas_brillig,and_the_slithy_toves,Did_gyre_and_gimble_in_the_wabe,All_mimsy_were_the_borogoves,And_the_mome-raths_outgrabe.' > /tmp/$pid.lq

	assert {~ `` () {cat /tmp/$pid.lw} `` '' {cat /tmp/$pid.lq}} \
		expected long string and long word to be identical
	let (x = `{wc -c /tmp/$pid.lw})
		assert {~ $x(1) 1088} expected long word to be 1088 bytes
	let (x = `{wc -c /tmp/$pid.lq})
		assert {~ $x(1) 1088} expected long quote to be 1088 bytes

	rm -f /tmp/$pid.lw /tmp/$pid.lq

	local (ifs = \n) {
		assert {~ `{echo h\
i} 'h i'} backslash-newline to space conversion
		assert {~ `{echo $es\\es} $es^\\es} backslash after variable name terminates variable name scan
		assert {~ `{echo $es\
es} $es^' es'} backslash-newline after variable name space conversion
		assert {~ `{echo h\\i} 'h\i'} backslash in the middle of word
		assert {~ `{echo h \\ i} 'h \ i'} free-standing backslash
	}

	assert {$es -c '# eof in comment'} 'eof in comment exits with zero status'
	assert {$es -c '{} $00'} '00 can be used as a variable'
}

test 'tokenizer errors' {
	local (ifs = '') {
		assert {~ `{$es -c 'echo hi |[2'	>[2=1]} *'expected ''='' or '']'' after digit'*}
		assert {~ `{$es -c 'echo hi |[92='	>[2=1]} *'expected digit or '']'' after ''='''*}
		assert {~ `{$es -c 'echo hi |[a]'	>[2=1]} *'expected digit after ''['''*}
		assert {~ `{$es -c 'echo hi |[2-]'	>[2=1]} *'expected ''='' or '']'' after digit'*}
		assert {~ `{$es -c 'echo hi |[2=99a]'	>[2=1]} *'expected '']'' after digit'*}
		assert {~ `{$es -c 'echo hi |[2=a99]'	>[2=1]} *'expected digit or '']'' after ''='''*}
		assert {~ `{$es -c 'echo ''hi'		>[2=1]} *'eof in quoted string'*}
	}
}

test 'blow the input stack' {
	assert {~ hi `{
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval echo hi
	}}
}

test 'umask' {
	local (tmp = /tmp/trip.$pid) {
		rm -f $tmp
		umask 0
		> $tmp
		let (x = `{ls -l $tmp})
			assert {~ $x(1) '-rw-rw-rw-'} umask 0 produced incorrect result: $x(1)

		rm -f $tmp
		umask 027
		> $tmp
		let (y = `{ls -l $tmp})
			assert {~ $y(1) '-rw-r-----'} umask 027 produced incorrect file: $y(1)

		rm -f $tmp
		assert {~ `umask 027 0027} fail umask reported bad value: `umask
	}

	let (exception = ()) {
		catch @ e {exception = $e} {umask bad}
		assert {~ $exception *'bad umask'*} '`umask bad` throws "bad umask" exception'
		exception = ()
		catch @ e {exception = $e} {umask -027}
		assert {~ $exception *'bad umask'*}'`umask -027` throws "bad umask" exception'
		exception = ()
		catch @ e {exception = $e} {umask 999999}
		assert {~ $exception *'bad umask'*} '`umask 999999` throws "bad umask" exception'
	}

	assert {~ `umask 027 0027} bad umask changed umask value to `umask
}

test 'redirections' {
	echo foo > foo > bar
	let (x = `{wc -c foo})
		assert {~ $x(1) 0} double redirection created non-empty empty file
	let (y = `{wc -c bar})
		assert {~ $y(1) 4} double redirection created wrong sized file: $y(1)
	rm -f foo bar

	echo -n >1 >[2]2 >[1=2] foo
	assert {~ `` '' {cat 1} ()} dup created non-empty empty file: `` '' {cat 1}
	assert {~ `` '' {cat 2} foo} dup put wrong contents in file : `` '' {cat 2}
	rm -f 1 2

	assert {~ `` \n {$es -c 'cat >[0=]' >[2=1]} *'closing standard input: Bad file descriptor'*}
	assert {~ `` \n {$es -c 'cat >(1 2 3)' >[2=1]} *'too many'*}
	assert {~ `` \n {$es -c 'cat >()' >[2=1]} *'null'*}
}

test 'exceptions' {
	assert {~ `` '' {
		let (x = a b c d e f g)
		catch @ e {
			echo caught $e
			if {!~ $#x 0} {
				x = $x(2 ...)
				throw retry
			}
			echo never succeeded
		} {
			echo trying ...
			eval '@'
			echo succeeded -- something''''s wrong
		}
	} \
'trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
never succeeded
'
	}
}

test 'heredocs and herestrings' {
	let (
		bigfile = /tmp/big.$pid
	) {
		od $es | sed 5q > $bigfile
		let (
			abc = (this is a)
			x = ()
			result = 'this is a heredoc
this is an heredoc
'
		) {
			assert {~ `` '' {<<[5] EOF cat <[0=5]} $result} 'unquoted heredoc'
$abc heredoc$x
$abc^n $x^here$x^doc
EOF
		}
		assert {~ `` \n cat '	'} 'quoted heredoc' << ' '
	
 
		<<<[9] `` '' {cat $bigfile} \
		{
			assert {~ `` '' {cat <[0=9]} `` '' cat} 'large herestrings'
		} < $bigfile

		rm -f $bigfile
	}

	assert {~ `{cat<<eof
$$
eof
	} '$'} 'quoting ''$'' in heredoc'

	assert {~ `` \n {$es -c 'cat<<eof' >[2=1]} *'pending'*}
	assert {~ `` \n {$es -c 'cat<<eof'\n >[2=1]} *'incomplete'*}
	assert {~ `` \n {$es -c 'cat<<eof'\n\$ >[2=1]} *'incomplete'*}

	assert {~ `` \n {$es -c 'cat<<()' >[2=1]} *'not a single literal word'*}
	assert {~ `` \n {$es -c 'cat<<(eof eof)' >[2=1]} *'not a single literal word'*}
	assert {~ `` \n {$es -c 'cat<<'''\n''''\n >[2=1]} *'contains a newline'*}
}

test 'tilde matching' {
	assert {$es -c '~ 0 1 `{}`{}`{}`{}`{} 0'}
	assert {$es -c '! ~ `{} 1 `{}`{}`{}`{}`{} 0'}
	assert {$es -c '~~ 0 1 `{}`{}`{}`{}`{}`{}`{}`{}`{} 0'}
}

test 'flat command expansion' {
	let (x = `^{echo some random phrase that should be identified as a single string})
		assert {~ $#x 1}
	let (x = `^{echo simple test with concatenation}^' '^`^{echo another random phrase})
		assert {~ $#x 1}
	let (x = ``^ abc {echo -n abchello})
		assert {~ $x 'hello'}

	assert {~ `` \n {$es -c '``^{true}' >[2=1]} *'syntax error'*}
	assert {~ `` \n {$es -c '`^^{true}' >[2=1]} *'syntax error'*}
}

# Redundant with tests/embedded-eq.es
test 'equal sign in command arguments' {
	assert {$es -c 'echo foo=bar' > /dev/null} '''='' in argument does not cause error'
	assert {~ `^{echo foo=bar} 'foo=bar'} '''='' is automatically concatenated with adjacent strings'
	assert {$es -c 'echo foo = bar' > /dev/null} '''='' as standalone argument does not cause error'
	assert {~ `^{echo foo = bar} 'foo = bar'} '''='' is not automatically concatenated with non-adjacent strings'
	assert {~ `` \n {$es -c 'foo^= = 384; echo $foo'} *'= 384'*}
	assert {~ `` \n {$es -c 'echo =foo; echo $echo'} *'foo'*}
}
