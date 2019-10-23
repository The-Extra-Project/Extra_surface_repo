import org.apache.hadoop.conf.Configuration
import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.fs.Path
val hadoopConf = new Configuration()
val hdfs = FileSystem.get(hadoopConf)

val vv1 : Value = List("p 1 0 f /home/lcaraffa/shared_spark/input/pts_generated_id0_19_10_2018_09_11_36.pts",
                       "p 1 1 s /home/lcaraffa/shared_spark/input/pts_generated_id0_19_10_2018_09_11_36.pts",
                       "p 1 2 f /home/lcaraffa/shared_spark/input/pts_generated_id0_19_10_2018_09_11_36.pts")

def get_fpath_value(vdat : VData) : Char = {
  var cc : Char = ' ' 
  var acc = 1;
  while (! { cc=vdat.charAt(acc); cc }.isLetter){
    acc = acc  +1
  }
  return cc
}

def is_data_file(vdat: VData) : Boolean = {
  return (get_fpath_value(vdat) == 'f')
}


def copy_data_to_local(vdat : VData,output_dir:String){
if(is_data_file(vdat)){
    vdat.split(" ").last
}else{
 vdat
}
}

def copy_kvrdd_to_local(vrdd: Value,output_dir:String){
   val res = vrdd.map(copy_data_to_local(_,output_dir)
   print(res)
}
}
  //hdfs.copyToLocalFile(false,new Path("/tmp/test_cp.txt"),new Path("/tmp/hoho.txt"), true);


