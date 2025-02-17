<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
   <xsl:template match="/">
   <table width="97%" border="0" cellpadding="0" cellspacing="0">
      <xsl:for-each select="//sample" order-by="type; name">
         <tr>
         <td valign="top" width="100%">
            
            <xsl:attribute name="class">
            <xsl:choose>
               <xsl:when test="@installed[.='no']">name_notinstalled</xsl:when>
               <xsl:otherwise>name_installed</xsl:otherwise>
            </xsl:choose>
            </xsl:attribute>

            <xsl:value-of select="name"/>

            
            <img class="language" hspace="2" width="57" height="17">
            <xsl:choose>
               <xsl:when test="language[.='C++']"><xsl:attribute name="src">img/lang_cpp.gif</xsl:attribute></xsl:when>
               <xsl:when test="language[.='Managed']"><xsl:attribute name="src">img/lang_managed.gif</xsl:attribute></xsl:when>
               <xsl:otherwise><xsl:attribute name="src">img/spacer.gif</xsl:attribute></xsl:otherwise>
            </xsl:choose>
            </img>
            <nobr class="dateformat"><xsl:if test="yearadded[.!='']">(
            <xsl:choose>
            <xsl:when test="monthadded[.=1]">January </xsl:when>
            <xsl:when test="monthadded[.=2]">February </xsl:when>
            <xsl:when test="monthadded[.=3]">March </xsl:when>
            <xsl:when test="monthadded[.=4]">April </xsl:when>
            <xsl:when test="monthadded[.=5]">May </xsl:when>
            <xsl:when test="monthadded[.=6]">June </xsl:when>
            <xsl:when test="monthadded[.=7]">July </xsl:when>
            <xsl:when test="monthadded[.=8]">August </xsl:when>
            <xsl:when test="monthadded[.=9]">September </xsl:when>
            <xsl:when test="monthadded[.=10]">October </xsl:when>
            <xsl:when test="monthadded[.=11]">November </xsl:when>
            <xsl:when test="monthadded[.=12]">December </xsl:when>
            </xsl:choose><xsl:value-of select="yearadded"/>)</xsl:if></nobr>
         </td>
         <td valign="top" align="right">
            <table cellpadding="0" cellspacing="0" border="0">
            <tr>
                <td>
                    <xsl:choose>
                       <xsl:when test="@beta[.='true']"><img class="language" width="44" height="20"><xsl:attribute name="src">img/results_beta.gif</xsl:attribute></img></xsl:when>
                       <xsl:when test="@preview[.='true']"><img class="language" width="44" height="20"><xsl:attribute name="src">img/results_preview.gif</xsl:attribute></img></xsl:when>
                    </xsl:choose>
                    <img class="language" hspace="8" width="44" height="20">
                    <xsl:choose>
                    <xsl:when test="@updated[.='true']"><xsl:attribute name="src">img/results_updated.gif</xsl:attribute></xsl:when>
                       <xsl:when test="@new[.='true']"><xsl:attribute name="src">img/results_new.gif</xsl:attribute></xsl:when>
                       <xsl:otherwise><xsl:attribute name="src">img/spacer.gif</xsl:attribute></xsl:otherwise>
                    </xsl:choose>
                    </img>
                </td>
               <td>
                  <img class="difficulty" width="90" height="19" hspace="10">
                  <xsl:choose>
                     <xsl:when test="complexity[. > 1999]"><xsl:attribute name="src">img/diff_advanced.gif</xsl:attribute></xsl:when>
                     <xsl:when test="complexity[. > 999]"><xsl:attribute name="src">img/diff_intermediate.gif</xsl:attribute></xsl:when>
                     <xsl:when test="complexity[. > 0]"><xsl:attribute name="src">img/diff_beginner.gif</xsl:attribute></xsl:when>
                     <xsl:otherwise><xsl:attribute name="src">img/spacer.gif</xsl:attribute></xsl:otherwise>
                  </xsl:choose>
                  </img>
               </td>
               <td>
               <img class="difficulty" width="90" height="19">
                  <xsl:choose>
                     <xsl:when test="type[.='sample']"><xsl:attribute name="src">img/type_sample.gif</xsl:attribute></xsl:when>
                     <xsl:when test="type[.='tutorial']"><xsl:attribute name="src">img/type_tutorial.gif</xsl:attribute></xsl:when>
                     <xsl:when test="type[.='article']"><xsl:attribute name="src">img/type_article.gif</xsl:attribute></xsl:when>
                     <xsl:when test="type[.='utility']"><xsl:attribute name="src">img/type_utility.gif</xsl:attribute></xsl:when>
                     <xsl:when test="type[.='utilitysample']"><xsl:attribute name="src">img/type_utilitysample.gif</xsl:attribute></xsl:when>
                  </xsl:choose>
               </img>
               </td>
            </tr>
            </table>
         </td>
         <td rowspan="4" valign="top" align="right">
            <xsl:attribute name="class">
            <xsl:choose>
               <xsl:when test="screenshot[.!='']">Screenshot</xsl:when>
               <xsl:otherwise>noScreenshot</xsl:otherwise>
            </xsl:choose>
            </xsl:attribute>

            <img hspace="10" width="78" height="12" style="position: relative; top: -12px; left: -10px;">
            <xsl:choose>
               <xsl:when test="@installed[.='no']"><xsl:attribute name="src">img\results_not_installed.gif</xsl:attribute></xsl:when>
               <xsl:when test="@external[.='true']"><xsl:attribute name="src">img\results_external.gif</xsl:attribute></xsl:when>
               <xsl:otherwise><xsl:attribute name="src">img\spacer.gif</xsl:attribute></xsl:otherwise>
            </xsl:choose>
            </img>

            
            <xsl:if test="screenshot[.!='']">
            <img width="90" border="0" onError="style.display = 'None'" style="position: relative; top: -5px">
               <xsl:attribute name="src">
               ..\..\<xsl:value-of select="folder"/><xsl:value-of select="screenshot"/>
               </xsl:attribute>
            </img>
            </xsl:if>
            
         </td>
         </tr>
         <tr>
         <td colspan="2" valign="top"><p class="description_highlight"></p></td>
         </tr>
         <tr>
         <td colspan="2" valign="top"><p class="description"><xsl:value-of select="description"/></p><p></p></td>
         </tr>
         <tr>
         <td colspan="2" valign="top" height="30" style="padding-left: 22px">
            <xsl:if test="exe[.!='']">
               <xsl:choose>
                  <xsl:when test="@onX64[.='yes']">
                     <a class="links">
                        <xsl:if test="doc[.!='']"><xsl:attribute name="onclick">if (showDoc) window.location = document.getElementById("doc<xsl:value-of select="name"/><xsl:value-of select="language"/>")</xsl:attribute></xsl:if>
                        <xsl:if test="readme[.!='']"><xsl:attribute name="onclick">if (showDoc) window.location = document.getElementById("doc<xsl:value-of select="name"/><xsl:value-of select="language"/>")</xsl:attribute></xsl:if>
                        <xsl:choose>
                           <xsl:when test="consoleapp[.='yes']">
                              <xsl:attribute name="href">DXEXECMDCONSOLE:<xsl:value-of select="exefolder"/>x86\<xsl:value-of select="exe"/>"</xsl:attribute>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="exefolder"/>x86\<xsl:value-of select="exe"/>"</xsl:attribute>
                           </xsl:otherwise>
                        </xsl:choose>
                        <xsl:attribute name="title">..\..\<xsl:value-of select="exefolder"/>x86\<xsl:value-of select="exe"/></xsl:attribute>32-bit&#160;Executable<font style="text-decoration: none">&#160;</font></a>
                     <xsl:choose>
                        <xsl:when test="@x64[.='false']"></xsl:when>
                        <xsl:otherwise>
                           <a class="links">
                              <xsl:if test="doc[.!='']"><xsl:attribute name="onclick">if (showDoc) window.location = document.getElementById("doc<xsl:value-of select="name"/><xsl:value-of select="language"/>")</xsl:attribute></xsl:if>
                              <xsl:if test="readme[.!='']"><xsl:attribute name="onclick">if (showDoc) window.location = document.getElementById("doc<xsl:value-of select="name"/><xsl:value-of select="language"/>")</xsl:attribute></xsl:if>
                              <xsl:choose>
                                 <xsl:when test="consoleapp[.='yes']">
                                    <xsl:attribute name="href">DXEXECMDCONSOLE:<xsl:value-of select="exefolder"/>x64\<xsl:value-of select="exe"/>"</xsl:attribute>
                                 </xsl:when>
                                 <xsl:otherwise>
                                    <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="exefolder"/>x64\<xsl:value-of select="exe"/>"</xsl:attribute>
                                 </xsl:otherwise>
                              </xsl:choose>
                              <xsl:attribute name="title">..\..\<xsl:value-of select="exefolder"/>x64\<xsl:value-of select="exe"/></xsl:attribute>64-bit&#160;
                              Executable<font style="text-decoration: none">&#160;</font></a>
                        </xsl:otherwise>
                     </xsl:choose>
                  </xsl:when>
                  <xsl:otherwise>
                     <a class="links">
                        <xsl:if test="doc[.!='']"><xsl:attribute name="onclick">if (showDoc) window.location = document.getElementById("doc<xsl:value-of select="name"/><xsl:value-of select="language"/>")</xsl:attribute></xsl:if>
                        <xsl:if test="readme[.!='']"><xsl:attribute name="onclick">if (showDoc) window.location = document.getElementById("doc<xsl:value-of select="name"/><xsl:value-of select="language"/>")</xsl:attribute></xsl:if>
                        <xsl:choose>
                           <xsl:when test="consoleapp[.='yes']">
                              <xsl:attribute name="href">DXEXECMDCONSOLE:<xsl:value-of select="exefolder"/>x86\<xsl:value-of select="exe"/></xsl:attribute>
                           </xsl:when>
                           <xsl:otherwise>
                              <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="exefolder"/>x86\<xsl:value-of select="exe"/></xsl:attribute>
                           </xsl:otherwise>
                        </xsl:choose>
                        <xsl:attribute name="title">..\..\<xsl:value-of select="exefolder"/>x86\<xsl:value-of select="exe"/></xsl:attribute>
                        Executable<font style="text-decoration: none">&#160;</font></a>                  
                  </xsl:otherwise>
               </xsl:choose>
            </xsl:if>
            
            <xsl:if test="doc[.!='']">
            <a class="links">
               <xsl:attribute name="id">doc<xsl:value-of select="name"/><xsl:value-of select="language"/></xsl:attribute>
               <xsl:attribute name="href">DXHELPCMD:<xsl:value-of select="doc"/></xsl:attribute>
               <xsl:attribute name="title"><xsl:value-of select="doc"/></xsl:attribute>
               Documentation<font style="text-decoration: none">&#160;</font></a>
            </xsl:if>

            <xsl:if test="readme[.!='']">
               <a class="links">
               <xsl:attribute name="id">doc<xsl:value-of select="name"/><xsl:value-of select="language"/></xsl:attribute>        
               <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="folder"/><xsl:value-of select="readme"/></xsl:attribute>
               <xsl:attribute name="title">..\..\<xsl:value-of select="folder"/><xsl:value-of select="readme"/></xsl:attribute>
               Documentation<font style="text-decoration: none">&#160;</font></a>
            </xsl:if>

            <xsl:if test="movie[.!='']">
               <a class="links">
               <xsl:attribute name="id">doc<xsl:value-of select="name"/><xsl:value-of select="language"/></xsl:attribute>        
               <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="folder"/><xsl:value-of select="movie"/></xsl:attribute>
               <xsl:attribute name="title">..\..\<xsl:value-of select="folder"/><xsl:value-of select="movie"/></xsl:attribute>
               Prerendered&#160;Movie<font style="text-decoration: none">&#160;</font></a>
            </xsl:if>

            <xsl:if test="PDF[.!='']">
               <a class="links">
               <xsl:attribute name="id">doc<xsl:value-of select="name"/><xsl:value-of select="language"/></xsl:attribute>        
               <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="folder"/><xsl:value-of select="PDF"/></xsl:attribute>
               <xsl:attribute name="title">..\..\<xsl:value-of select="folder"/><xsl:value-of select="PDF"/></xsl:attribute>
               Adobe&#160;PDF&#160;Format<font style="text-decoration: none">&#160;</font></a>
            </xsl:if>

            <xsl:if test="XLS[.!='']">
               <a class="links">
               <xsl:attribute name="id">doc<xsl:value-of select="name"/><xsl:value-of select="language"/></xsl:attribute>        
               <xsl:attribute name="href">DXEXECMD:<xsl:value-of select="folder"/><xsl:value-of select="XLS"/></xsl:attribute>
               <xsl:attribute name="title">..\..\<xsl:value-of select="folder"/><xsl:value-of select="XLS"/></xsl:attribute>
               Excel&#160;Format<font style="text-decoration: none">&#160;</font></a>
            </xsl:if>

            <xsl:choose>
            <xsl:when test="install">
               <a class="links">
               <xsl:attribute name="href">DXINSTALLCMD:<xsl:value-of select="folder"/>
                  <xsl:if test="install/@name[.!='']">/NAME<xsl:value-of select="install/@name"/></xsl:if>
                  <xsl:if test="install/@common[.='false']">/NOCOMMON</xsl:if>
                  <xsl:if test="install/@dxut[.='true']">/DXUT</xsl:if>
                  <xsl:for-each select="install/reserved">/RESERVED<xsl:value-of/></xsl:for-each>
                  <xsl:for-each select="install/media">/MEDIA<xsl:value-of/></xsl:for-each>
                  <xsl:for-each select="install/additional">/ADDITIONAL<xsl:value-of select="@src" />:<xsl:value-of select="@dest" /></xsl:for-each>
                  <xsl:for-each select="install/createfile">/CREATEFILE<xsl:value-of select="@size" />:<xsl:value-of/></xsl:for-each>
               </xsl:attribute>
               Install&#160;Project<font style="text-decoration: none">&#160;</font></a>
            </xsl:when>
            </xsl:choose>

            
            <xsl:if test="link[.!='']">
            <a class="links">
               <xsl:attribute name="href"><xsl:value-of select="link"/></xsl:attribute>
               <xsl:attribute name="title"><xsl:value-of select="link"/></xsl:attribute>
               <xsl:attribute name="target">new</xsl:attribute>
               <xsl:value-of select="link/@name"/><font style="text-decoration: none">&#160;</font></a>
            </xsl:if>

            <a class="links">
               <xsl:attribute name="href">javascript:sendfeedback('<xsl:value-of select="name"/>','<xsl:value-of select="language"/>', '<xsl:value-of select="type"/>')</xsl:attribute>
               Feedback<font style="text-decoration: none">&#160;</font></a>        
         </td>
         </tr>
      </xsl:for-each>
   </table>
   </xsl:template>
</xsl:stylesheet>
