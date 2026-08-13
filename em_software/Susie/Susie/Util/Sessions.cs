using System;
using System.Collections.Generic;
using System.Text;

namespace Susie.Util
{
    public class Sessions
    {        
        //Session state
        private enum Session_state
        {
            inactive,
            active,
            paused,
            ended
        }


        //Start date and time
        private DateTime start;
        //duration (s)
        private int duration;
        //current session state
        Session_state state;

        public Sessions()
        {
            start = DateTime.Now;
            state = Session_state.inactive;
        }
    }
}
