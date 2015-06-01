using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;

namespace MorphExtractorCL
{
    public class SwcHeaderStore
    {
        private OrderedDictionary tables;

        public SwcHeaderStore()
        {
            tables = new OrderedDictionary();

            tables["ORIGINAL_SOURCE"] = "SIGEN";
            tables["CREATURE"] = "";
            tables["REGION"] = "";
            tables["FIELD/LAYER"] = "";
            tables["TYPE"] = "";
            tables["CONTRIBUTOR"] = "";
            tables["REFERENCE"] = "";
            tables["RAW"] = "";
            tables["EXTRAS"] = "";
            tables["SOMA_AREA"] = "";
            tables["SHINKAGE_CORRECTION"] = "";
            tables["VERSION_NUMBER"] = "";
            tables["VERSION_DATE"] = "";
            tables["SCALE"] = "";
            tables["INTERPOLATION"] = "";
            tables["DISTANCE_THRESHOLD"] = "";
            tables["VOLUME_THRESHOLD"] = "";
            tables["SMOOTHING"] = "";
            tables["CLIPPING"] = "";
            tables["ROOT_SETTING"] = "NO";
        }

        public IEnumerable<string> PublishHeader()
        {
            var list = new List<string>();

            foreach (object key in tables.Keys)
            {
                object value = tables[key];
                if (value.ToString().Length > 0)
                {
                    list.Add("# " + key + " " + value);
                }
                else
                {
                    list.Add("# " + key);
                }
            }

            return list;
        }

        public void Update(string key, string value)
        {
            tables[key] = value;
        }

        public void InitBySwcComments(IEnumerable<string> comments)
        {
            foreach (var line in comments)
            {
                string commentRemoved = line.TrimStart('#', ' ');
                IEnumerable<string> record = commentRemoved.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if (record.Count() > 0)
                {
                    string key = record.First();
                    string value = string.Join(" ", record.Skip(1).ToArray());
                    tables[key] = value;
                }
            }
        }

        public void SetRootSetting(bool yesOrNo)
        {
            tables["ROOT_SETTING"] = (yesOrNo ? "YES" : "NO");
        }

        public void SetSmoothing(int level)
        {
            tables["SMOOTHING"] = level;
        }

        public void SetClipping(int level)
        {
            tables["CLIPPING"] = level;
        }

        public void SetInterpolation(bool yesOrNo)
        {
            tables["INTERPOLATION"] = (yesOrNo ? "YES" : "NO");
        }

        public void SetDistanceThreshold(double r)
        {
            tables["DISTANCE_THRESHOLD"] = r;
        }

        public void SetVolumeThreshold(double v)
        {
            tables["VOLUME_THRESHOLD"] = v;
        }

        public void SetScale(double x, double y, double z)
        {
            tables["SCALE"] = string.Format("{0} {1} {2}", x, y, z);
        }

        public void SetSource(string name)
        {
            tables["RAW"] = name;
        }

        public void SetVersionDate()
        {
            string dateStr = DateTime.Now.ToString("yyyy-MM-dd");
            tables["VERSION_DATE"] = dateStr;
        }
    }
}
