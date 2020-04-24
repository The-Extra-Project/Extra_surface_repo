import convertcloud as cvc

conv = cvc.Converter()
conv.load_points("/home/laurent/shared_spark/inputs/daratech/daratech_20.npts")
conv.set_ori_dist(5);
conv.convert("/home/laurent/shared_spark/inputs/daratech_good/daratech_20.ply")


