# Introduce
Named-Thread(Render Thread & Logic Thread), Trible Buffers. 

## Still In progress

A lot artifact still in project, try my best to fixing.

## Flags
  ✅ PCSS 

  ✅ Defered Rendering

  ✅ SSAO 

  ❎ SSR   

  ❎ PBR

## Snapshot
PCSS:
![PCSS](snapshoot/pcss/pcss.png)

Defered:
![Defered](snapshoot/defered//defered.png)

CSM:
![Cascade Shdowmap](snapshoot/cascade_shadowmap/cascade.png)
![Cascade Shdowmap](snapshoot/cascade_shadowmap/csm.png)

SSAO:
![SSAO](snapshoot/ssao/ssao.png)

## Tips

- CreateSet and Bind is disastor in Vulkan, try to make a static function when your want a block data and bind later.

## Problems

- Multi VkPass is disastor in Tiled-GPU.
- Everywhere maybe optimizable.