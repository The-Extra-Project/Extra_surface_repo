 jq '. | to_entries | map(select(.key | match("spark_core_max";"i"),match("final";"i"))) |  map(.value)  ' *.json
 jq '. | to_entries | map(select(.key | match("nbp";"i"))) | map(.value) ' application_1617724072015_0257_27_05_2021_05_14_46_austin.params.json
 jq '. | to_entries | map(select(.key | match("spark_core_max";"i"),match("final";"i"))) |  sort_by(.spark_core_max) |  map(.value|tonumber)   | @csv ' *.json
