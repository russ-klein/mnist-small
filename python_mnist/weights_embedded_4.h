 
   static const int   base_address              = 0x40000000; 
 
      
   static const int layer1_input_images         = 1;  
   static const int layer1_output_images        = 8;  
   static const int layer1_weights_rows         = 3;  
   static const int layer1_weights_cols         = 3;  
      
   static const int layer1_num_weights          = 72;  
   static const int layer1_unit_size            = 9;  
   static const int layer1_unit_offset_factor   = 3;  
   static const int later1_unit_count           = 8;  
      
   static const int layer1_weight_offset           = 0;  
   static const int layer2_weight_offset        = layer1_weight_offset + 24;  
      
      
      
   static const int layer2_input_images         = 8;  
   static const int layer2_output_images        = 3;  
   static const int layer2_weights_rows         = 3;  
   static const int layer2_weights_cols         = 3;  
      
   static const int layer2_num_weights          = 216;  
   static const int layer2_unit_size            = 9;  
   static const int layer2_unit_offset_factor   = 3;  
   static const int later2_unit_count           = 24;  
      
   static const int layer3_weight_offset        = layer2_weight_offset + 72;  
      
      
      
   static const int layer3_weights_rows = 10; 
   static const int layer3_weights_cols = 2352; 
      
   static const int layer3_num_weights        = 23520;  
   static const int layer3_unit_size          = 2352;  
   static const int layer3_unit_offset_factor = 588;  
   static const int later3_unit_count         = 10;  
      
   static const int layer4_weight_offset      = layer3_weight_offset + 5880;  
      
      
 
   static const int   image_height              = 28; 
   static const int   image_width               = 28; 
 
   static const int   top_of_weights            = 5976; 
 
