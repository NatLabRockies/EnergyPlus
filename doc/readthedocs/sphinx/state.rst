State API
=========

What is this `state` business?
------------------------------

OK, so you are using this API and may have seen `state` as an argument floating around the API functions (or inside the code if you looked close enough).
Up until recently, it was standard practice in EnergyPlus to put all the data in the global state of the program.
Because the program was always run one way - a single exe thread - this was not a problem because the data was deleted when the program ended.

EnergyPlus is venturing into diverse applications and workflows, including running as a library, as a service, embedded in controllers, used with hardware in the loop and other research experimentation
For these applications, having a huge blob of unmanaged global data is a problematic design for multiple reasons:

#. The data in the program is not cleared when you call EnergyPlus as a library multiple times in the same thread.
   The second time a run is started, previous simulation data will be present if the data is not cleared.
   While this does not explicitly require moving away from global data, having an unmanaged blob of thousands of global variables to reset is a maintenance nightmare.
#. For some of those applications, it's not just about clearing the global state, but also being able to save and reload the state of the program in order to be used as part of control decisions will be important.
#. It is impossible for EnergyPlus to run multi-threaded in the same process, because the global variables are not thread-safe (and adding thread-local to thousands of global variables is a no-go for performance reasons).
   With many new workflows, on machines that have plenty of CPU cores available, running EnergyPlus as part of a multi-threaded library will be heavily beneficial.

So what does this have to do with this new `state` variable I've seen around?
Think of this as the instance of EnergyPlus that you are currently running.
In a fully OO design, this could just be the object instance that you create and then execute methods on that instance.
EnergyPlus is quickly moving away from global state, and into "managed" state, but some global data still remains in the program.
This state object holds a reference to the ever growing "managed" state, which will eventually encapsulate the entire running state of the program.

From an API client perspective, nothing ever needs to be manually done with this object.
Simply create the instance of state and pass it into all the API functions and let EnergyPlus do what it needs to do.
API callback functions receive a payload of the running state instance so that it can again be passed into API functions within the callback.
Python plugin override methods also accept a state instance argument so that API function calls can be made with the running state.
State really is just a small payload that needs to be properly passed around through the API.


.. automodule:: state
    :members:
    :undoc-members:
    :show-inheritance:

