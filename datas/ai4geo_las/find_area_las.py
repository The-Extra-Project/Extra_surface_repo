filepath = './19LE31100_AWiart.prj'
xmin_ref = 574000.0000
ymin_ref = 6280000.0000
xmax_ref = 574500.0000
ymax_ref = 6280500.0000

rr = 500

ll_name = []
with open(filepath) as fp:
   line = fp.readline()
   cnt = 0
   fname= ""
   xmin = 0
   xmax = 0
   ymin =  0
   ymax =  0
   acc = 0
   while line:
      acc +=1
      line = fp.readline()
      line_st = line.strip()
      if acc <= 32 :
         continue;
      
      val_mod = cnt%9
      print("lint_st:" + line_st)
      if val_mod == 0 :
         fname = line_st.replace("Block","")
      elif val_mod == 3 :
         xmin = float(line_st.split()[0])
         ymin = float(line_st.split()[1])
      elif val_mod == 5 :
         xmax = float(line_st.split()[0])
         ymax = float(line_st.split()[1])
      if val_mod == 6 :
         if(xmin >=  xmin_ref - rr and
            ymin >=  ymin_ref - rr and
            xmax <=  xmax_ref + rr and
            ymax <=  ymax_ref + rr) :
            ll_name.append(fname)
            print("take:" + fname)
            print(str(xmin) + " " + str(xmax) + " " + str(ymin) +  " " + str(ymax))
      cnt += 1

print(ll_name)
