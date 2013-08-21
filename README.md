axe v0.1
========

A tool for hacking the ENT files on StarMade servers. Currently the tool supports the following 

   * Printing out the almost complete structure of any ent file
   * Modifying (print, set, add, multiply) the credits of playerstate type ents. 
   * Modifying (print, set, add, multiply) inventory blocks of playerstate and shops type ents. 

StarMade version: 0.09387

Audience
--------

The tool is intended mainly for scripting and server owners, therefore it will not have any interactive operations at all and all features are inteded to provide _expressibility_ rather than convinience. 

Future other tools can provide operations more suited for end-users.

TODO
----
 - [ ] Get more TODO inputs from the users.
 - [ ] Print out sector/spawn information from players.
 - [ ] Allow the set/add/mult operations to work on every stack of an item of the same id. Currently only the first stack is affected.
 - [ ] Tidy up datastructure after removing entires. Stacks of size zero is currently left, and removed when the user logs in. 


Commandline options
-------------------

These are the valid commandline operations and options. Everything that is not an operation/option is considered to be an ent file and will be processed. 

 * `--print-all` Print data-tree for ENT file. Will abort if unhandled datatypes exists in files.
 * `--print-credits` Print credits for playerstate type ENT files
 * `--set-credits <integer>` Sets the credits of playerstate type ENT files
 * `--add-credits <integer>` Adds a given amount of credits (can be negative) to playerstate type ENT files
 * `--mult-credits <float>` Multiplies credits by a constant
 * `--print-block <id>` Prints number of blocks matching the ID that is found in the playerstate OR shop type ENT files
 * `--set-block <id> <integer>` Sets the number of blocks of given ID that is found in the playerstate OR shop type ENT files
 * `--add-block <id> <integer>` Adds the number of blocks of given ID (can be negative)
 * `--mult-block <id> <float>` Multiplies the number of blocks of given ID in the playerstate OR shop type ENT files
 * `--simulate` Prevents any changes from beeing written to disk
 * `--quiet` Suppresses any informational printouts to simplify script processing

Examples
--------

*Note* all examples must be run when the server is offline. Also, make backup! This is very much alpha software still and it *probably* will fail and break stuff for you!

Example of how to give/set the number of thruster blocks a user has.
    
    mathias@ygdrasil:~/games/star-made/StarMade/server-database$ ~/development/axe/axe --set-block 8 ENTITY_PLAYERSTATE_User-1.ent
    

Example of making every player on the server loose a tiny fraction of money due to inflation
    
    mathias@ygdrasil:~/games/star-made/StarMade/server-database$ ~/development/axe/axe --mult-credits 0.99 *STATE_User-1.ent
    

Example of restocking shops, so that they will smoothly get SOME more power blocks but will become saturated at 1000 blocks. 
    
    mathias@ygdrasil:~/games/star-made/StarMade/server-database$ ~/development/axe/axe --mult-blocks 2 0.5 --add-blocks 2 500 *SHOP*.ent
    
For the math: assume that the shop has X items of the given kind, then we will keep 0.5X + 500. If the shop has more than 1000 items then we will have a net loss of items, if it has less than 1000 items we will get new items.
If you want to make a smoothing forumla that is slower you can do: `--mult-blocks <id> <speed> --add-blocks <id> <magic>` where _speed_ is a value from 0.0 to 1.0, and where _magic_ is the limit you want the items to saturate at multiplied by 1-speed. 

Example of how to print all information contained in an ent file. Note that bytearray objects are skipped (I can later add options for exporting them if requested). Any raw serialized java objects will abort the program. 

    mathias@ygdrasil:~/games/star-made/StarMade/server-database$ ~/development/axe/axe --print-all ENTITY_PLAYERSTATE_User-1.ent
    ENTITY_PLAYERSTATE_User-1.ent
    {
      struct PlayerState : {
        int credits : 18701
        float3 spawn : 0.000000 0.000000 0.000000
        struct inventory : {
          list slots : [8] { 
            int 0 : 1
            int 1 : 12
            int 2 : 9
            int 3 : 10
            int 4 : 3
            int 5 : 2
            int 6 : 11
            int 7 : 0
          }
          list types : [8] { 
            short 0 : 2
            short 1 : 5
            short 2 : -32
            short 3 : 122
            short 4 : 6
            short 5 : 8
            short 6 : 336
            short 7 : 16
          }
          struct values : {
            int null : 101
            int null : 24
            struct null : {
              int null : 101
              short null : -32
              struct null : {
                int null : 25349
                float null : 0.000000
                int null : 150
              }
              short null : 1
            }
            int null : 4
            int null : 1
            int null : 12
            int null : 4
            int null : 17
          }
        }
        int3 sector : 2 2 2
        float3 lspawn : 96001544.000000 0.000000 6788076915156233868260518914252865536.000000
        int3 lsector : 2 2 2
        struct pFac-v0 : {
          int null : 0
        }
        long null : 1377035067317
        long null : 1377035121718
        struct ips : {
          null : /127.0.0.1
        }
        bytearray ci0 : <binary size 256>
      }
    }

Example of 
    
/ Mathias Broxvall (Hari_Seldon @mpnetwork, @brierie)


