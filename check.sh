#!/bin/bash
SRCDIR=_posts

for file in "${SRCDIR}"/*; do
        NEW=`head -10 "${file}" | grep -cE '^(tags|tag):'`
        if [[ ${NEW} -le 0 ]]; then
                continue
        fi

        ADS=`grep -cE ' include ads_content..\.html ' "${file}"`
        if [[ ${ADS} -le 0 ]]; then
                echo "${file}"
        fi
done
