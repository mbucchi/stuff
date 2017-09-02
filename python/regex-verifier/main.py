__author__ = 'mabucchi'
import argparse
import re
from collections import defaultdict


def memoize(func):
    results = dict()

    def new_func(*args):
        if args not in results:
            results[args] = func(*args)
        return results[args]

    return new_func

# utility function
def to_regex(regexp, idx=0):
    '''Transforms a prefix regular expression to an infix expression'''
    curr = regexp[idx]
    if curr == '+':
        fst, idx = to_regex(regexp, idx + 1)
        snd, idx = to_regex(regexp, idx)
        return '(' + fst + '+' + snd + ')', idx
    elif curr == '*':
        res, idx = to_regex(regexp, idx + 1) 
        return '(' + res + ')*', idx
    elif curr == '.':
        fst, idx = to_regex(regexp, idx + 1)
        snd, idx = to_regex(regexp, idx)
        return fst + snd, idx
    elif curr == '\\':
        idx += 1
        curr = regexp[idx]
    return curr, idx+1


# utility function
def to_transitions(regexp, idx=0, curr_state=0):
    '''Transfroms a prefix regular expression into a string that defines an automata.
    This string can then be used to instantiate an NFA that will accept strings that match the given regexp.'''
    curr = regexp[idx]
    if curr == '+':
        start_a = curr_state + 1
        t1, idx, end_a = to_transitions(regexp, idx + 1, start_a)
        start_b = end_a + 1
        t2, idx, end_b = to_transitions(regexp, idx + 1, start_b)
        end = end_b + 1
        enter = 'transition {curr} {start_a}\ntransition {curr} {start_b}\n'.format(
            curr=curr_state,
            start_a=start_a,
            start_b=start_b)
        out = 'transition {end_a} {end}\ntransition {end_b} {end}\n'.format(end_a=end_a,
                                                                            end_b=end_b,
                                                                            end=end)
        return enter + t1 + t2 + out, idx, end

    elif curr == '*':
        start = curr_state + 1
        t, idx, end = to_transitions(regexp, idx + 1, start)
        enter = 'transition {curr} {start}\ntransition {curr} {end}\ntransition {end} {start}\n'.format(
            curr=curr_state,
            start=start,
            end=end)
        return enter + t, idx, end

    elif curr == '.':
        start_a = curr_state + 1
        t1, idx, end_a = to_transitions(regexp, idx + 1, start_a)
        start_b = end_a + 1
        t2, idx, end_b = to_transitions(regexp, idx + 1, start_b)
        enter = 'transition {curr} {start_a}\ntransition {end_a} {start_b}\n'.format(curr=curr_state,
                                                                                     start_a=start_a,
                                                                                     end_a=end_a,
                                                                                     start_b=start_b)
        return enter + t1 + t2, idx, end_b

    elif curr == '\\':
        idx += 1
        curr = regexp[idx]

    return 'transition {} {} {}\n'.format(curr_state, curr, curr_state + 1), idx, curr_state + 1


class NFA:
    '''This class represents a non-deterministic finite automaton. Once instantiated, it can run over a string returning True if the string matches the regular expression and False if it does not.'''
    def __init__(self, states, transitions, init, final):
        self.states = states
        self.transitions = transitions
        self.e_jumps = defaultdict(lambda: set())
        self.init = init
        self.final = final
        self.word = None

    @classmethod
    def from_file(cls, file_name):
        with open(file_name, "r") as file:
            lines = file.readlines()
        return cls._make(lines)

    @classmethod
    def from_regex(cls, regexp):
        transitions, _, final = to_transitions(regexp)
        info = transitions + 'accept {}\ninit 0'.format(final)
        return cls._make(info.split('\n'))

    @classmethod
    def _make(cls, transition_list):
        obj = cls(set(), defaultdict(lambda: set()), set(), set())
        alphabet = set()

        info_re = re.compile('(\d+) (. )?(\d+)')

        for line in transition_list:
            try:
                _type, info = re.split("\s", line.strip(), 1)
            except:
                obj.word = ''
            if _type == 'transition':
                s1, char, s2 = info_re.match(info).groups()
                if char is None:
                    obj.e_jumps[s1].add(s2)
                    obj.states.update({s1, s2})
                else:
                    char = char[0]
                    obj.states.update({s1, s2})
                    obj.transitions[(s1, char)].add(s2)
                    alphabet.add(char)
            elif _type == 'accept':
                obj.final.add(info)
            elif _type == 'init':
                obj.init.add(info)
            else:
                obj.word = info
        return obj

    @memoize
    def transition(self, states, char):
        new_states = set()
        states = set(states)
        prev = 0
        while len(states) != prev:
            prev = len(states)
            new = set()
            for state in states:
                new.update(self.e_jumps[state])
            states.update(new)
        for state in states:
            new_states.update(self.transitions[(state, char)])
        return frozenset(new_states)

    def execute(self, word=None):
        if word is None:
            word = self.word

        current = frozenset(self.init)

        for char in word:
            current = self.transition(current, char)

        final = set(current)
        prev = 0
        while len(final) != prev:
            prev = len(final)
            new = set()
            for state in final:
                new.update(self.e_jumps[state])
            final.update(new)

        if final & self.final:
            return True
        return False


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Compile a regular expression and try to match a string.')
    parser.add_argument('regexp', help='Regular expression to compile')
    parser.add_argument('string', help='The string to test the regular expression over')

    args = parser.parse_args()
    nfa = NFA.from_regex(args.regexp)
    result = nfa.execute(args.string)
    print("Word '{}'{} accepted.".format(args.string, "" if result else " NOT"))