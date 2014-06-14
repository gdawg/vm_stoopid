# vm_stoop(id)

It's vm_stat, but stoopider.

Instead of "Pages free" - "Memory free", "Pages active" - "Memory active".

    $ vm_stoopid|=> vm_stoop f
    Memory free: 6779M
    $ vm_stoopid|=> vm_stoop a
    Memory active: 3820M
    $ vm_stoopid|=> vm_stoop fau
    Memory free: 6780M Memory active: 3820M Memory purgeable:  350M

