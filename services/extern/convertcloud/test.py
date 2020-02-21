import convertcloud as cvc

conv = cvc.Converter()
conv.load_points("crocodile_statue_config_0.npts")
conv.set_ori_dist(5);
conv.convert("converted.ply")


