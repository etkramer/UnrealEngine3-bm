<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
   <xsl:template match="/">
      <xsl:for-each select="//sample" order-by="type; name">
            <xsl:choose>
            <xsl:when test="install">
               SILENTINSTALL:<xsl:value-of select="folder"/>
                  <xsl:if test="install/@name[.!='']">/NAME<xsl:value-of select="install/@name"/></xsl:if>
                  <xsl:if test="install/@common[.='false']">/NOCOMMON</xsl:if>
                  <xsl:if test="install/@dxut[.='true']">/DXUT</xsl:if>
                  <xsl:for-each select="install/reserved">/RESERVED<xsl:value-of/></xsl:for-each>
                  <xsl:for-each select="install/media">/MEDIA<xsl:value-of/></xsl:for-each>
                  <xsl:for-each select="install/additional">/ADDITIONAL<xsl:value-of select="@src" />:<xsl:value-of select="@dest" /></xsl:for-each>
                  <xsl:for-each select="install/createfile">/CREATEFILE<xsl:value-of select="@size" />:<xsl:value-of/></xsl:for-each>
            </xsl:when>
            </xsl:choose>
      </xsl:for-each>
   </xsl:template>
</xsl:stylesheet>
