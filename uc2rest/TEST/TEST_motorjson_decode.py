import json

mString = '''
++
{
	"success":	1
}

--
++
{
        "steppers":     [{
                        "stepperid":    3,
                        "position":     -2000,
                        "isDone":       1
                }]
}
--
'''

# Split the string into lines and filter out only the JSON lines
json_lines = [line for line in mString.strip().split('\n') if line not in ('++', '--')]

# Join the filtered lines to form a valid JSON string
json_str = "".join(json_lines)

# Add brackets to make it a JSON array and parse
json_array = json.loads('[' + json_str + ']')

# Now, json_array contains the two objects as a list of dictionaries
for obj in json_array:
    print(obj)
