module cat_accel (                                                     
  input          clock,                                                
  input          resetn,                                               
  input  [15:0]  read_addr,                                            
  output [63:0]  read_data,                                            
  input          oe,                                                   
  input  [15:0]  write_addr,                                           
  input  [63:0]  write_data,                                           
  input  [7:0]   be,                                                   
  input          we                                                   
);                                                                     
                                                                       
                                                                       
 reg     [63:0]  register_bank[15:0];                                  
 reg     [63:0]  rd_reg;                                               
                                                                       
 reg             ready_out = 1'b1;                                     
 reg             resp_out = 2'b00;                                     
                                                                       
 wire    [63:0]  read_address;                                         
 wire    [63:0]  write_address;                                        
 wire            read_enable = oe;                                     
 wire            write_enable = we;                                    
                                                                       
 assign read_data = rd_reg;                                            
                                                                       
 assign read_address = read_addr[15:0];                                
 assign write_address = write_addr[15:0];                              
                                                                       
 always @(posedge clock or resetn == 1'b0) begin                       
   if (resetn == 1'b0) begin                                           
     rd_reg <= 32'h00000000;                                           
   end else begin                                                      
     if (read_enable) begin                                            
       rd_reg <= register_bank[read_address];                          
     end                                                               
   end                                                                 
 end                                                                   
                                                                       
 always @(posedge clock or resetn == 1'b0) begin                       
   if (resetn == 1'b0) begin                                           
       register_bank[0] <= 32'h00000000; 
   end else begin                                                      
     if (write_enable) begin                                           
       if (write_address < 16) begin                                   
         register_bank[write_address] <= write_data;                   
       end                                                             
     end                                                               
   end                                                                 
 end                                                                   
                                                                       
 endmodule 
