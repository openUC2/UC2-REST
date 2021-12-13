class openapi:
    endpoint = None 
    response = None
    variables = None
    
    def __init__(self):
        pass
    

class INO2OpenAPI:
    
    def __init__(self, filename):
        self.filename=filename
        self.type_list = ['int', 'char', 'float', 'double', 'bool', 'void', 'short', 'long', 'signed', 'struct']

        
        api = openapi()

        # extract all functions from code
        self.all_fcts  =  fnel.func_name_extract(self.filename)
        
    def convert(self):
        # 1. extract all possible endpoints
        self._extract_endpoints()
        
        # 2. get relevant information from function that is called by an endpoint
        N_endpoints = len(self.endpoints)
        print("Found Endoints: "+str(N_endpoints))
        
        self.apidefiitions = []
        # iterate over all endpoints        
        for iendpoints in range(N_endpoints):
            fct_info = self._extract_fct(self.endpoints[iendpoints]["fct_endpoint"])
            
            # 3. extract response codes
            response = self._extract_response(fct_info)
            
            # 4. extrac variables
            variables = self._extract_variables(fct_info)
            
            # construct api            
            api = openapi()
            api.endpoint = self.endpoints[iendpoints]
            api.response = response 
            api.variables = variables
            
            self.apidefiitions.append(api)
            
    def _extract_endpoints(self):
        # extract HTTP endpoints from INO file
        search_key = "server.on("
        with open(filename, 'r') as f:
            self.line_keys = f.readlines()

        endpoints = []
        
        for iline in range(len(line_keys)):
            linecontent = line_keys[iline]
            if linecontent.find(search_key)>=0 and linecontent.find("Swagger")<0:
                # store endpoints in dictionary
                fct_endpoint = linecontent.split(",")[-1].split(")")[0].replace(" ","")
                http_method = linecontent.split(",")[-2].replace(" ","").replace("HTTP_","").lower()
                endpoint = linecontent.split(",")[0].replace("server.on(","").replace("'","").replace("\"","")
                endpoints.append({"linecontent": linecontent,
                                  "linenumber": iline, 
                                  "http_method": http_method,
                                  "fct_endpoint": fct_endpoint, 
                                  "endpoint": endpoint})
                
        self.endpoints = endpoints 
        
    def _extract_fct(self, endpoint):
        # extract functions that point to an endpoint
        fct_code = [] 
        for ifct in range(len(self.all_fcts)):
            if endpoint in self.all_fcts[ifct]:
                start = self.all_fcts[ifct][1]
                end = self.all_fcts[ifct][2]
                # put togehter code for one function
                for iline in  range(start,end):
                    fct_code.append(line_keys[iline])
        return fct_code
        
    def _extract_response(self, fct_code):
        # extract response        
        for iline in range(len(fct_code)):
            # scan for relevant retur messages
            
            if fct_code[iline].find("server.send(")>=0:
                # eg server.send(200, "application/json", answer);
                response = fct_code[iline]
                response_code = response.split(",")[0].split("(")[-1]
                response_type = response.split(",")[1].replace(" ","")
                response_val = response.split(",")[-1].split(")")[0].replace(" ","")
                
                resopnse_return = {'response_code': response_code,
                    'response_type': response_type,
                    'response_val': response_val}
                return resopnse_return
            
            
    def _extract_variables(self, fct_code):
        #extrac values that can be altered 
        variable_strings = [] 
        for iline in range(len(fct_code)):
            # scan for relevant retur messages
            try:
                variable_string = fct_code[iline]
                variable_string = re.search(r"\[\"([A-Za-z0-9_]+)\"\]", variable_string).group(1)

                for vtype in self.type_list:
                    if fct_code[iline].find(vtype)>=0:
                        variable_type = vtype
                
                variable = {"variable_strig": variable_string,
                            "variable_type": variable_type}
                
                variable_strings.append(variable)
            except Exception as  e :
                pass #print(e)

        resopnse_return = {'variables': variable_strings}
        return resopnse_return
                        