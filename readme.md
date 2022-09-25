# In progress

Named-Thread(Render Thread & Logic Thread), Trible Buffers.

## Flags
  ✅ PCSS 

  ✅ Defered Rendering

  ❎ SSR   

  ❎ SSAO 

  ❎ PBR

## Snapshot
PCSS:
![PCSS](snapshoot/pcss/pcss.png)


Defered:
![Defered](snapshoot/defered//defered.png)

CSM:
![Cascade Shdowmap](snapshoot/cascade_shadowmap/cascade.png)
![Cascade Shdowmap](snapshoot/cascade_shadowmap/csm.png)

## Tips

- CreateSet and Bind is disastor in Vulkan, try to make a static function when your want a block data and bind later.