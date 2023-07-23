#!/bin/bash

export_script="$1"
if [ -f "$export_script" ]; then

    # canonicalize path
    export_script=$(readlink -f "$export_script")

    base_dir=$(dirname "$export_script")
    root_dir=$(dirname $(dirname "$base_dir"))

    product_export_scripts=$(cat "$export_script" | grep -w "source" | cut -d" " -f2)

    for product in $product_export_scripts; do
        detected_product_dir=$(dirname "$product")
        detected_root_dir=$(dirname $(dirname "$detected_product_dir"))
        corrected_product=${product/$detected_root_dir/$root_dir}

        cat "$corrected_product" | sed "s@${detected_root_dir}@${root_dir}@g"
    done

else
    2>&1 echo "WARNING: export script does not exist: $export_script"
fi
