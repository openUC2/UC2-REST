#%%
from swagger_parser import SwaggerParser

myfile = '/Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/UC2-REST/SWAGGER/openapi_default.json'

parser = SwaggerParser(swagger_path=myfile)  # Init with file

#%%
# Get an example of dict for the definition Foo
parser.definitions_example.get('pet')
