#ifndef Z3DSHADERGROUP_H
#define Z3DSHADERGROUP_H

#include <map>
#include "z3dshaderprogram.h"
#include "z3drendererbase.h"

class Z3DShaderGroup
{
public:
  Z3DShaderGroup();
  ~Z3DShaderGroup();

  void init(const QStringList &shaderFiles, const QString &header, Z3DRendererBase *base,
            const QStringList &normalShaderFiles = QStringList());
  void removeAllShaders();
  void addAllSupportedPostShaders();
  void addDualDepthPeelingShaders();
  void addWeightedAverageShaders();

  void bind();
  void release();
  Z3DShaderProgram& get();
  void rebuild(const QString &header);

private:
  void buildNormalShader(Z3DShaderProgram *shader);
  void buildDualDepthPeelingInitShader(Z3DShaderProgram *shader);
  void buildDualDepthPeelingPeelShader(Z3DShaderProgram *shader);
  void buildWeightedAverageShader(Z3DShaderProgram *shader);

private:
  QStringList m_shaderFiles;
  QString m_header;
  Z3DRendererBase* m_base;
  QStringList m_normalShaderFiles;
  std::map<Z3DRendererBase::ShaderHookType, Z3DShaderProgram*> m_shaders;
};

#endif // Z3DSHADERGROUP_H
