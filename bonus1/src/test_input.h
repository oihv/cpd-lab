const char test_input[][24 * 64] = {
    "q0 q1 q2 \n0 1 \nq0\n\nq1 q2\nq0 0 q0 q1 \nq0 1 q0 \nq1 0 q2 \nq1 1 "
    "- \nq2 0 q2 \nq2 1 q1",

    "q0 q1 q2 q3\n0 1\nq0\nq3\nq0 0 q0 q1\nq0 1 q0\nq1 0 q2\nq1 1 "
    "q1\nq2 0 q3\nq2 1 q2\nq3 0 q3\nq3 1 q3",
    "q0 q1 q2\na b\nq0\nq2\nq0 a q1\nq0 b q0\nq1 a q2\nq1 b q1\nq2 a "
    "q2\nq2 b q2",
    "q0 q1\n0 1\nq0\nq1\nq0 0 q0 q1\nq0 1 q0\nq1 0 -\nq1 1 q1",
    "q0 q1 q2\n0 1\nq0\nq2\nq0 0 q0 q1\nq0 1 q0\nq1 0 q2\nq1 1 -\nq2 "
    "0 q2\nq2 1 q1",
    "q0 q1 q2 q3\neps 0 1\nq0\nq3\nq0 0 q2\nq0 1 -\nq0 eps q1\nq1 0 "
    "-\nq1 1 q2 \nq1 eps q3\nq2 eps -\nq2 0 q2 \nq2 1 q3 \nq3 1 q2\nq3 "
    "0 -\nq3 eps -",
    "A B C D E F\n0 1 eps\nB\nD\nA 0 E\nA 1 B\nB 1 C\nB eps D\nC 1 "
    "D\nE 0 F\nE eps B C\nF 0 D",
};

char test_input_desc[] = "nfa1;nfa2;nfa3;nfa4;nfa5;e-nfa1;e-nfa2";

size_t test_input_count = 7;
