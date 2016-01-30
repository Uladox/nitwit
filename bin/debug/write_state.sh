#! /bin/bash

# printf '\x03' >> state.ilk
rm state.ilk

printf '\x0C' >> state.ilk #NTWT_OP_INIT_PACK
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x01' >> state.ilk

printf '\x03' >> state.ilk #NTWT_OP_TEST

printf '\x02' >> state.ilk #NTWT_OP_CONTEXT
printf '\x00' >> state.ilk

printf '\x05' >> state.ilk #NTWT_OP_ACTION
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk
printf '\x00' >> state.ilk

printf '\x03' >> state.ilk #NTWT_OP_TEST

printf '\x09' >> state.ilk
printf '\x01' >> state.ilk
